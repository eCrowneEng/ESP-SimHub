# SimHub Protocol Documentation

This document describes the serial protocol used between SimHub and ESP-SimHub devices
(ESP32 / ESP8266). It covers the ARQ transport layer, the SimHub command layer, device-to-host
output framing, ESP-NOW wireless framing, and the TCP/WiFi bridge.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Transport Layer — ARQ Serial Protocol](#2-transport-layer--arq-serial-protocol)
   - [Packet Structure](#21-packet-structure)
   - [CRC-8 Checksum](#22-crc-8-checksum)
   - [Packet Sequencing](#23-packet-sequencing)
   - [Acknowledgements](#24-acknowledgements)
   - [Timeouts](#25-timeouts)
3. [Application Layer — SimHub Commands](#3-application-layer--simhub-commands)
   - [Command Frame Format](#31-command-frame-format)
   - [Command Reference](#32-command-reference)
   - [Extended Commands (X)](#33-extended-commands-x)
   - [Motor Sub-Commands (V)](#34-motor-sub-commands-v)
4. [Device-to-Host Output Framing](#4-device-to-host-output-framing)
5. [Custom Packets (Device-Initiated)](#5-custom-packets-device-initiated)
6. [Feature String](#6-feature-string)
7. [Baud Rate Codes](#7-baud-rate-codes)
8. [Communication Flow Examples](#8-communication-flow-examples)
9. [ESP-NOW Wireless Protocol](#9-esp-now-wireless-protocol)
10. [TCP/WiFi Bridge](#10-tcpwifi-bridge)
11. [Constants Summary](#11-constants-summary)

---

## 1. Overview

SimHub communicates with hardware devices over a serial link (USB, WiFi virtual COM port,
or ESP-NOW). The protocol has two layers:

- **Transport layer** — An ARQ (Automatic Repeat Request) framing scheme that adds packet
  IDs and CRC-8 error detection so that corrupted or out-of-order frames can be detected
  and retransmitted. Implemented in `src/ArqSerial.h`.

- **Application layer** — A simple command-response protocol carried inside ARQ packets.
  SimHub sends single-character commands preceded by a header byte (`0x03`). The device
  responds with typed output frames. Implemented in `src/SHCommands.h` and `src/main.cpp`.

Three connection types are supported:

| Constant   | Value  | Description                            |
|------------|--------|----------------------------------------|
| `SERIAL`   | `0b00` | USB / hardware UART                    |
| `WIFI`     | `0b01` | WiFi TCP bridge (e.g. Perle TruePort)  |
| `ESP_NOW`  | `0b10` | Wireless ESP-NOW peer-to-peer          |

Default baud rate: **19200**.
Firmware revision identifier: **`'j'`** (defined as `VERSION` in `src/main.cpp`).

---

## 2. Transport Layer — ARQ Serial Protocol

Source: `src/ArqSerial.h`

### 2.1 Packet Structure

Every packet sent **from SimHub to the device** has this layout:

```
+--------+--------+-----------+--------+------- - - ------+-------+
|  0x01  |  0x01  | Packet ID | Length |    Data bytes    |  CRC8 |
| 1 byte | 1 byte |  1 byte   | 1 byte |  1 – 32 bytes    | 1 byte|
+--------+--------+-----------+--------+------- - - ------+-------+
```

| Field       | Size    | Description                                                    |
|-------------|---------|----------------------------------------------------------------|
| Header      | 2 bytes | Always `0x01 0x01`. Marks the start of a packet.              |
| Packet ID   | 1 byte  | Sequence number 0–127, then wraps to 0. `0xFF` = broadcast.   |
| Length      | 1 byte  | Number of data bytes that follow. Must be 1–32.               |
| Data        | 1–32 B  | Payload — one or more SimHub command bytes.                    |
| CRC8        | 1 byte  | Checksum over Packet ID + Length + all Data bytes.             |

**Example — Hello command (`0x03 0x31 0x10`):**

```
01 01 FF 03 03 31 10 6A
└──┘ └──┘ ─── ─── └──────┘ ───
 hdr  hdr  ID len  data    CRC
```

- Header: `01 01`
- Packet ID: `FF` (broadcast)
- Length: `03` (3 data bytes)
- Data: `03 31 10` → header byte `0x03`, command `'1'` (`0x31`), end byte `0x10`
- CRC8: `6A`

### 2.2 CRC-8 Checksum

The checksum is calculated using a 256-entry CRC-8 lookup table stored in program memory
(`PROGMEM`). The calculation covers Packet ID, Length, and all Data bytes:

```c
currentCrc = 0;
currentCrc = updateCrc(currentCrc, packetID);
currentCrc = updateCrc(currentCrc, length);
for (i = 0; i < length; i++) {
    currentCrc = updateCrc(currentCrc, data[i]);
}
```

where `updateCrc(crc, byte)` = `crc_table_crc8[crc ^ byte]`.

### 2.3 Packet Sequencing

The device tracks the last successfully received Packet ID in `Arq_LastValidPacket`
(initialised to `255` / broadcast).

The expected next Packet ID is:

```
nextPacketId = (Arq_LastValidPacket > 127) ? 0 : Arq_LastValidPacket + 1;
```

A received packet is accepted when:
- Its Packet ID equals `nextPacketId`, **or**
- Its Packet ID is `0xFF` (broadcast — always accepted, sequence not advanced).

If neither condition is met the packet is silently discarded (but an ACK is still sent to
keep SimHub's retransmit logic happy).

### 2.4 Acknowledgements

After every received packet the device sends one of two responses directly on the serial
stream (outside ARQ framing):

**ACK** — packet accepted:

```
03 <Packet ID>
```

**NACK** — packet rejected:

```
04 <Last valid Packet ID> <Reason>
```

Reason codes:

| Code   | Meaning                            |
|--------|------------------------------------|
| `0x01` | Bad / missing Packet ID            |
| `0x02` | Bad Length (≤ 0 or > 32)          |
| `0x03` | CRC byte missing / unreadable      |
| `0x04` | CRC mismatch                       |
| `0x05` | Data byte(s) missing / unreadable  |

### 2.5 Timeouts

| Timeout          | Value   | Scope                                   |
|------------------|---------|-----------------------------------------|
| Per-byte read    | 50 ms   | Single `Arq_TimedRead()` call           |
| Whole-read block | 400 ms  | `arqserial.read()` — waiting for data   |

---

## 3. Application Layer — SimHub Commands

Source: `src/SHCommands.h`, `src/main.cpp`

### 3.1 Command Frame Format

Inside the ARQ data payload SimHub uses a simple command frame:

```
MESSAGE_HEADER  Command  [Optional data bytes...]
    0x03         1 byte
```

The device's main loop checks for `MESSAGE_HEADER` (`0x03`), reads the next byte as the
command character, then dispatches to the appropriate handler.

### 3.2 Command Reference

| Cmd | ASCII | Handler                   | Device Response                              |
|-----|-------|---------------------------|----------------------------------------------|
| `1` | `'1'` | `Command_Hello`           | Firmware version char (e.g. `'j'`)           |
| `0` | `'0'` | `Command_Features`        | Feature string + `\n` (see §6)               |
| `4` | `'4'` | `Command_RGBLEDSCount`    | 1-byte LED count                             |
| `2` | `'2'` | `Command_TM1638Count`     | 1-byte TM1638 module count                   |
| `B` | `'B'` | `Command_SimpleModulesCount` | 1-byte 7-segment module count            |
| `A` | `'A'` | `Command_Acq`             | `0x03` (keep-alive ACK)                      |
| `N` | `'N'` | `Command_DeviceName`      | Device name string + `\n`                    |
| `I` | `'I'` | `Command_UniqueId`        | MAC address string + `\n`                    |
| `J` | `'J'` | `Command_ButtonsCount`    | 1-byte total button count                    |
| `X` | `'X'` | Extended command dispatch | See §3.3                                     |
| `3` | `'3'` | `Command_TM1638Data`      | *(none — reads display data from stream)*    |
| `V` | `'V'` | `Command_Motors`          | See §3.4                                     |
| `S` | `'S'` | `Command_7SegmentsData`   | *(none — reads display data from stream)*    |
| `6` | `'6'` | `Command_RGBLEDSData`     | `0x15` after processing                      |
| `R` | `'R'` | `Command_RGBMatrixData`   | `0x15` after processing                      |
| `M` | `'M'` | `Command_MatrixData`      | *(none)*                                     |
| `G` | `'G'` | `Command_GearData`        | *(none — reads single gear char from stream)*|
| `L` | `'L'` | `Command_I2CLCDData`      | *(none)*                                     |
| `K` | `'K'` | `Command_GLCDData`        | *(none — OLED / Nokia LCD)*                  |
| `P` | `'P'` | `Command_CustomProtocolData` | `0x15` after processing                  |
| `8` | `'8'` | `Command_SetBaudrate`     | *(none — see §7)*                            |

### 3.3 Extended Commands (X)

When command byte is `'X'`, the device reads a space- or newline-terminated ASCII string
from the stream and dispatches on it:

```
0x03  'X'  <subcommand string>  ' ' or '\n'
```

| Sub-command string  | Handler                    | Response                                    |
|---------------------|----------------------------|---------------------------------------------|
| `list`              | `Command_ExpandedCommandsList` | Newline-separated list of supported sub-commands |
| `mcutype`           | `Command_MCUType`          | 3 raw bytes: `SIGNATURE_0 SIGNATURE_1 SIGNATURE_2` |
| `tach`              | `Command_TachData`         | Tachometer data (if enabled)                |
| `speedo`            | `Command_SpeedoData`       | Speedometer data (if enabled)               |
| `boost`             | `Command_BoostData`        | Boost gauge data (if enabled)               |
| `temp`              | `Command_TempData`         | Temperature gauge data (if enabled)         |
| `fuel`              | `Command_FuelData`         | Fuel gauge data (if enabled)                |
| `cons`              | `Command_ConsData`         | Consumption gauge data (if enabled)         |
| `encoderscount`     | `Command_EncodersCount`    | 1-byte encoder count (if enabled)           |

MCU signature bytes (emulating Arduino Mega ATmega2560):

| Byte          | Value  |
|---------------|--------|
| `SIGNATURE_0` | `0x1E` |
| `SIGNATURE_1` | `0x98` |
| `SIGNATURE_2` | `0x01` |

### 3.4 Motor Sub-Commands (V)

The `'V'` command reads one additional byte to determine the motor sub-operation:

| Sub-byte | Meaning             | Response                                                   |
|----------|---------------------|------------------------------------------------------------|
| `'C'`    | Count / capability  | `0xFF`, motor count byte, provider name strings + `\n`    |
| `'S'`    | Set motor values    | *(none — reads motor data bytes from stream)*              |

---

## 4. Device-to-Host Output Framing

All data sent back to SimHub is prefixed with a type marker byte. These frames are sent
**outside** the ARQ packet wrapper.

### 4.1 Single Byte

```
0x08  <byte>
```

Used for 1-byte responses such as counts or ACK values.

### 4.2 String (with optional newline)

```
0x06  <length>  <string bytes>  0x20
```

- `length` includes the `\n` byte when `PrintLn` variants are used.
- `0x20` (space) is the frame terminator.

Example — sending `"j"` (firmware version):

```
06 01 6A 20
```

### 4.3 Debug Message

```
0x07  <length>  <string bytes>  0x20
```

Same structure as a string frame. Used for `DebugPrintLn` / `DebugPrint` calls.

### 4.4 Custom / Device-Initiated Packet

```
0x09  <packet type>  <length>  <data bytes>
```

Sent asynchronously by the device for encoder movements, button events, etc. (see §5).

---

## 5. Custom Packets (Device-Initiated)

The device can push events to SimHub at any time using `arqserial.CustomPacketStart()`.

### Encoder Movement

Sent when a rotary encoder position changes (direction 0 or 1):

```
0x09  0x01  0x03  <encoderId>  <direction>  <position>
```

| Byte       | Description                        |
|------------|------------------------------------|
| `0x09`     | Custom packet marker               |
| `0x01`     | Packet type — encoder move         |
| `0x03`     | Length = 3                         |
| encoderId  | 1-based encoder index              |
| direction  | `0` = clockwise, `1` = counter-CW  |
| position   | Absolute encoder position          |

Sent when encoder button is pressed/released (direction ≥ 2):

```
0x09  0x02  0x02  <encoderId>  <buttonState>
```

| Byte        | Description                           |
|-------------|---------------------------------------|
| `0x09`      | Custom packet marker                  |
| `0x02`      | Packet type — encoder button          |
| `0x02`      | Length = 2                            |
| encoderId   | 1-based encoder index                 |
| buttonState | direction − 2 (button press/release)  |

### Button Event

Sent when a standalone button changes state:

```
0x09  0x03  0x02  <buttonId>  <status>
```

| Byte     | Description                     |
|----------|---------------------------------|
| `0x09`   | Custom packet marker            |
| `0x03`   | Packet type — button event      |
| `0x02`   | Length = 2                      |
| buttonId | 1-based button index            |
| status   | `1` = pressed, `0` = released   |

### TM1638 Button Event

Sent when a TM1638 module button changes state:

```
0x09  0x04  0x03  <moduleIndex>  <buttonIndex>  <status>
```

| Byte        | Description                     |
|-------------|---------------------------------|
| `0x09`      | Custom packet marker            |
| `0x04`      | Packet type — TM1638 button     |
| `0x03`      | Length = 3                      |
| moduleIndex | 1-based TM1638 module index     |
| buttonIndex | 1-based button index (1–8)      |
| status      | `1` = pressed, `0` = released   |

---

## 6. Feature String

In response to command `'0'`, the device sends a concatenated string of single-character
capability codes followed by `\n`:

```
0x06  <length>  <feature chars...>  '\n'  0x20
```

| Code | Feature                                          | Condition                             |
|------|--------------------------------------------------|---------------------------------------|
| `M`  | LED matrix                                       | MAX7221/HT16K33 matrix enabled        |
| `L`  | I²C LCD                                          | `I2CLCD_enabled == 1`                 |
| `K`  | OLED or Nokia LCD                                | OLED or Nokia LCD count > 0           |
| `G`  | Gear display                                     | Always sent                           |
| `N`  | Named device (`Command_DeviceName` supported)    | Always sent                           |
| `I`  | Unique ID (`Command_UniqueId` supported)         | Always sent                           |
| `J`  | Button count query supported                     | Always sent                           |
| `P`  | Custom protocol supported                        | Always sent                           |
| `X`  | Extended commands supported                      | Always sent                           |
| `R`  | RGB matrix                                       | WS2812B/DM163/Sunfounder matrix > 0   |
| `V`  | Vibration / motor (ShakeIt)                      | Any motor driver enabled              |

Example response for a fully-featured device: `"MLKGNJPXRV\n"`.

---

## 7. Baud Rate Codes

Command `'8'` reads a 1-byte code from the stream and changes the serial baud rate:

| Code | Baud Rate  |
|------|------------|
| 1    | 300        |
| 2    | 1200       |
| 3    | 2400       |
| 4    | 4800       |
| 5    | 9600       |
| 6    | 14400      |
| 7    | 19200 *(default)* |
| 8    | 28800      |
| 9    | 38400      |
| 10   | 57600      |
| 11   | 115200     |
| 12   | 230400     |
| 13   | 250000     |
| 14   | 1000000    |
| 15   | 2000000    |
| 16   | 200000     |
| 17   | 500000     |

The device applies a 200 ms delay before switching baud rates to allow the host to follow.

---

## 8. Communication Flow Examples

### 8.1 Hello / Handshake

```
SimHub                                   Device
  |                                         |
  |--[01 01 FF 03  03 31 10  6A]----------->|   ARQ packet: Hello cmd (0x03 '1' 0x10)
  |                                         |
  |<--[03 FF]-------------------------------|   ACK for Packet ID FF
  |<--[06 01 6A 20]-------------------------|   String frame: firmware version 'j'
  |                                         |
```

### 8.2 Feature Query

```
SimHub                                   Device
  |                                         |
  |--[01 01 00 03  03 30 10  ??]----------->|   Cmd '0'
  |                                         |
  |<--[03 00]-------------------------------|   ACK
  |<--[06 0B 47 4E 4A 50 58 47 0A 20]------>|   Feature string "GNJPXG\n"
  |                                         |
```

### 8.3 RGB LED Update

```
SimHub                                   Device
  |                                         |
  |--[ARQ: 03 '6' <RGB data>]-------------->|
  |                                         |
  |<--[03 <ID>]------------------------------|   ACK
  |<--[08 15]--------------------------------|   Single-byte ACK 0x15
  |                                         |
```

### 8.4 Button Press (Device → Host)

```
SimHub                                   Device
  |                                         |
  |                        button pressed   |
  |<--[09 03 02 01 01]----------------------|   Custom packet: button 1 pressed
  |                                         |
```

### 8.5 NACK / Retransmit

```
SimHub                                   Device
  |                                         |
  |--[01 01 01 03  03 31 10  FF]----------->|   Corrupted CRC
  |                                         |
  |<--[04 00 04]----------------------------|   NACK: last valid=0x00, reason=0x04 (CRC)
  |                                         |
  |--[01 01 01 03  03 31 10  6A]----------->|   Retransmit with correct CRC
  |                                         |
  |<--[03 01]-------------------------------|   ACK
  |                                         |
```

---

## 9. ESP-NOW Wireless Protocol

Source: `lib/ESPNowSerialProtocol/EspNowProtocol.h`, `lib/ESPNowSerialBridge/ESPNowSerialBridge.h`

ESP-NOW uses a two-device setup:

- **Bridge device** — connected to the computer via USB. Forwards serial data from SimHub
  to the feature device over ESP-NOW, and returns responses.
- **Feature device** — runs the full SimHub firmware. Receives data via ESP-NOW and
  processes it exactly as if it were on a direct serial port.

### 9.1 ESP-NOW Message Struct

Each ESP-NOW transmission carries one `EspNowMessage` struct:

```c
typedef struct __attribute__((packed)) EspNowMessage {
    int  version;                          // MESSAGE_VERSION = 1
    int  length;                           // Actual bytes used in simHubBytes
    char simHubBytes[MAX_SIMHUB_BYTES];    // Up to 32 bytes of SimHub ARQ data
    char bridgeBytes[MAX_BRIDGE_BYTES];    // Up to 8 bytes of bridge control data
} EspNowMessage;
```

| Field          | Size    | Description                                   |
|----------------|---------|-----------------------------------------------|
| `version`      | 4 bytes | Protocol version, currently `1`               |
| `length`       | 4 bytes | Number of valid bytes in `simHubBytes`        |
| `simHubBytes`  | 32 bytes| SimHub ARQ packet data                        |
| `bridgeBytes`  | 8 bytes | Bridge control commands (see §9.2)            |

### 9.2 Bridge Command Format

Bridge commands are carried in `bridgeBytes` using this frame:

```
COMMAND_HEADER  <length>  <command>  <data bytes…>  COMMAND_END
    0x03         1 byte    1 byte     0–4 bytes       0x0A
```

| Constant         | Value  |
|------------------|--------|
| `COMMAND_HEADER` | `0x03` |
| `COMMAND_END`    | `0x0A` |

Supported bridge commands:

| Command byte | Code   | Description                                        |
|--------------|--------|----------------------------------------------------|
| `'8'`        | `0x38` | Change baud rate — same codes as §7. 3-byte value. |
| `'I'`        | `0x49` | Ping — keep-alive from bridge to feature device    |
| `'O'`        | `0x4F` | Pong — response to Ping                            |

**Example — Baudrate command (19200):**

```
03 03 38 00 4B 00 0A
│  │  │  └──────┘ │
│  │  │  19200LE  └─ end
│  │  └─ '8'
│  └─ length = 3
└─ header
```

---

## 10. TCP/WiFi Bridge

Source: `lib/TcpSerialBridge2/TcpSerialBridge2.h`

The WiFi bridge creates a transparent TCP socket on port `10001` (default). Perle TruePort
or similar software on the SimHub host creates a virtual COM port that forwards to this
socket, making the wireless device appear as a regular serial port to SimHub.

Configuration in `src/main.cpp`:

```c
#define CONNECTION_TYPE WIFI
#define BRIDGE_PORT 10001
#define USE_HARDCODED_CREDENTIALS false   // or true + WIFI_SSID / WIFI_PASSWORD
```

No changes to the SimHub command or ARQ layers are needed — the TCP bridge is transparent.

---

## 11. Constants Summary

### ARQ Transport

| Constant / Symbol           | Value    | Source           |
|-----------------------------|----------|------------------|
| ARQ header byte (×2)        | `0x01`   | `ArqSerial.h`    |
| ACK byte                    | `0x03`   | `ArqSerial.h`    |
| NACK byte                   | `0x04`   | `ArqSerial.h`    |
| Max packet data length      | 32 bytes | `ArqSerial.h`    |
| Broadcast Packet ID         | `0xFF`   | `ArqSerial.h`    |
| Per-byte read timeout       | 50 ms    | `ArqSerial.h`    |
| Block read timeout          | 400 ms   | `ArqSerial.h`    |

### Output Frame Markers

| Constant                  | Value  | Meaning              |
|---------------------------|--------|----------------------|
| Single-byte write marker  | `0x08` | 1-byte output        |
| String write marker       | `0x06` | String output        |
| Debug print marker        | `0x07` | Debug string output  |
| Custom packet marker      | `0x09` | Device-initiated pkt |
| String frame terminator   | `0x20` | End of string frame  |

### Application Layer

| Constant           | Value  | Source          |
|--------------------|--------|-----------------|
| `MESSAGE_HEADER`   | `0x03` | `SHCommands.h`  |
| `VERSION`          | `'j'`  | `main.cpp`      |
| `SIGNATURE_0`      | `0x1E` | `EspSimHub.h`   |
| `SIGNATURE_1`      | `0x98` | `EspSimHub.h`   |
| `SIGNATURE_2`      | `0x01` | `EspSimHub.h`   |
| Initial baud rate  | 19200  | `main.cpp`      |

### ESP-NOW

| Constant            | Value  | Source               |
|---------------------|--------|----------------------|
| `MESSAGE_VERSION`   | `1`    | `EspNowProtocol.h`   |
| `MAX_SIMHUB_BYTES`  | 32     | `EspNowProtocol.h`   |
| `MAX_BRIDGE_BYTES`  | 8      | `EspNowProtocol.h`   |
| `COMMAND_HEADER`    | `0x03` | `EspNowProtocol.h`   |
| `COMMAND_END`       | `0x0A` | `EspNowProtocol.h`   |
| `BAUDRATE_COMMAND`  | `'8'`  | `EspNowProtocol.h`   |
| `PING_COMMAND`      | `'I'`  | `EspNowProtocol.h`   |
| `PONG_COMMAND`      | `'O'`  | `EspNowProtocol.h`   |
