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
   - [Null Padding](#24-null-padding)
   - [Acknowledgements](#25-acknowledgements)
   - [Timeouts](#26-timeouts)
3. [Application Layer — SimHub Commands](#3-application-layer--simhub-commands)
   - [Command Frame Format](#31-command-frame-format)
   - [Command Reference](#32-command-reference)
   - [Extended Commands (X)](#33-extended-commands-x)
   - [Motor Sub-Commands (V)](#34-motor-sub-commands-v)
4. [Device-to-Host Output Framing](#4-device-to-host-output-framing)
5. [Custom Packets (Device-Initiated)](#5-custom-packets-device-initiated)
6. [Feature String](#6-feature-string)
7. [Baud Rate Codes](#7-baud-rate-codes)
8. [Session Lifecycle](#8-session-lifecycle)
   - [Phase 1 — Hello](#81-phase-1--hello)
   - [Phase 2 — Feature Enumeration](#82-phase-2--feature-enumeration)
   - [Phase 3 — Baud Rate Negotiation](#83-phase-3--baud-rate-negotiation)
   - [Phase 4 — Data Streaming](#84-phase-4--data-streaming)
9. [Communication Flow Examples](#9-communication-flow-examples)
10. [ESP-NOW Wireless Protocol](#10-esp-now-wireless-protocol)
11. [TCP/WiFi Bridge](#11-tcpwifi-bridge)
12. [Constants Summary](#12-constants-summary)

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

**Example — Hello command:**

```
01 01 FF 03 03 31 10 6A
└──┘ └──┘ ─── ─── └──────┘ ───
 hdr  hdr  ID len  data    CRC
```

- Header: `01 01`
- Packet ID: `FF` (broadcast — Hello is always sent as broadcast)
- Length: `03` (3 data bytes)
- Data: `03 31 10` → `MESSAGE_HEADER` `0x03`, command `'1'` (`0x31`), extra byte `0x10` consumed and discarded by `Command_Hello`
- CRC8: `6A`

**Verified init-sequence packets** (from captured traces):

| Command               | Full packet bytes                                            | ID   | Data                                  | CRC  |
|-----------------------|--------------------------------------------------------------|------|---------------------------------------|------|
| Hello (`'1'`)         | `01 01 FF 03 03 31 10 6A`                                    | `FF` | `03 31 10`                            | `6A` |
| Features (`'0'`)      | `01 01 00 02 03 30 38`                                       | `00` | `03 30`                               | `38` |
| RGB LED Count (`'4'`) | `01 01 01 02 03 34 83`                                       | `01` | `03 34`                               | `83` |
| TM1638 Count (`'2'`)  | `01 01 02 02 03 32 CD`                                       | `02` | `03 32`                               | `CD` |
| Simple Modules (`'B'`)| `01 01 03 02 03 42 E3`                                       | `03` | `03 42`                               | `E3` |
| X list                | `01 01 04 09 03 03 03 58 6C 69 73 74 0A 51`                  | `04` | `03 03 03 58 6C 69 73 74 0A`          | `51` |
| Device Name (`'N'`)   | `01 01 05 02 03 4E 7F`                                       | `05` | `03 4E`                               | `7F` |
| Unique ID (`'I'`)     | `01 01 06 02 03 49 E4`                                       | `06` | `03 49`                               | `E4` |
| Buttons Count (`'J'`) | `01 01 07 02 03 4A 0B`                                       | `07` | `03 4A`                               | `0B` |
| X mcutype             | `01 01 08 0C 03 03 03 58 6D 63 75 74 79 70 65 0A 78`         | `08` | `03 03 03 58 6D 63 75 74 79 70 65 0A` | `78` |
| Set Baud Rate (`'8'`) | `01 01 09 03 03 38 0B 6C`                                    | `09` | `03 38 0B`                            | `6C` |
| Gear (`'G'`)          | `01 01 0A 03 03 47 20 97`                                    | `0A` | `03 47 20`                            | `97` |
| X keepalive           | `01 01 0B 0C 03 58 6B 65 65 70 61 6C 69 76 65 0A 11`         | `0B` | `03 58 6B 65 65 70 61 6C 69 76 65 0A` | `11` |

Notes:
- Most simple commands carry 2 data bytes (`MESSAGE_HEADER` + command char).
- Hello carries 3 bytes — the extra `0x10` is consumed and discarded by `Command_Hello`.
- Baud rate, Gear, and similar commands carry 3 bytes — the third byte is the data argument.
- All X commands carry 12 bytes with the `03 03` no-op preamble (see §3.3).

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

Observed init sequence from captured traces: `FF` → `00` → `01` → `02` → `03` → `04` → …

### 2.4 Null Padding

In the raw byte stream (particularly over ESP-NOW) null bytes (`0x00`) may appear between
ARQ packets because ESP-NOW transmits fixed-size buffers; unused trailing bytes are zeroed.
The ARQ receiver safely ignores any byte that is not `0x01` when looking for the header,
so these zeros are silently skipped.

### 2.5 Acknowledgements

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

### 2.6 Timeouts

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

**Multiple commands per ARQ packet.** An ARQ payload can contain more than one command
frame. The device loops over the DataBuffer consuming command frames one at a time until
the buffer is empty.

**Unrecognised command bytes are silently discarded.** If the byte following `MESSAGE_HEADER`
does not match any `case` in the switch statement the device simply continues to the next
iteration. This is observable in the X command packet (see §3.3) where the payload starts
with a `03 03` pair (header + `0x03` command, which has no handler) followed by the real
command frame.

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

In practice SimHub packs the X command inside a 9-byte ARQ payload that begins with an
extra `03 03` no-op pair before the real command frame:

```
ARQ data: 03 03 03 58 6C 69 73 74 0A
          └──┘  └──┘  └──────────┘ └─ '\n' terminator
         no-op   'X'   "list"
```

The `03 03` bytes are processed first (header + unrecognised command `0x03`, discarded),
then `03 58` dispatches `'X'` with subcommand `"list\n"`.

| Sub-command string  | Handler                    | Response                                    | Always in `X list`? |
|---------------------|----------------------------|---------------------------------------------|---------------------|
| `list`              | `Command_ExpandedCommandsList` | String frame per sub-command + `08 0A` end | — (meta command)  |
| `mcutype`           | `Command_MCUType`          | `08 1E`, `08 98`, `08 01` (three single-byte frames) | Yes          |
| `keepalive`         | *(no handler)*             | ACK only — connection heartbeat             | Yes                 |
| `tach`              | `Command_TachData`         | Tachometer data (if enabled)                | If tachometer enabled |
| `speedo`            | `Command_SpeedoData`       | Speedometer data (if enabled)               | If speedo enabled   |
| `boost`             | `Command_BoostData`        | Boost gauge data (if enabled)               | If boost enabled    |
| `temp`              | `Command_TempData`         | Temperature gauge data (if enabled)         | If temp enabled     |
| `fuel`              | `Command_FuelData`         | Fuel gauge data (if enabled)                | If fuel enabled     |
| `cons`              | `Command_ConsData`         | Consumption gauge data (if enabled)         | If cons enabled     |
| `encoderscount`     | `Command_EncodersCount`    | 1-byte encoder count (if enabled)           | If encoders enabled |

`keepalive` and `mcutype` are always emitted by `Command_ExpandedCommandsList` regardless
of device configuration. `keepalive` has no handler in the device firmware — SimHub sends
it continuously during the data streaming phase as a heartbeat; the device returns only
the ARQ ACK.

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

Used for single-byte responses: counts, version character, ACK values. Examples from
captured traces:

| Response          | Bytes     | Meaning                              |
|-------------------|-----------|--------------------------------------|
| Hello version     | `08 6A`   | Firmware version `'j'` (0x6A)        |
| RGB LED count     | `08 00`   | 0 RGB LEDs configured                |
| Command ACK       | `08 03`   | `Command_Acq` response (`0x03`)      |

### 4.2 String (with optional newline)

```
0x06  <length>  <string bytes>  0x20
```

- `length` includes the `\n` byte when `PrintLn` variants are used.
- `0x20` (space) is the frame terminator.
- Each call to `FlowSerialPrint()` / `FlowSerialPrintLn()` produces exactly one frame.
  Multi-character strings are sent as one frame; single characters produce a 4-byte frame.

Example — one feature code `'G'` (0x47):

```
06 01 47 20
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

In response to command `'0'`, the device sends each feature code as a **separate** string
frame (`0x06` marker), one character per frame. The sequence is terminated by a final
frame containing only `'\n'`. There is no single combined frame for the whole string.

Each frame follows the standard string format:

```
0x06  0x01  <feature char>  0x20
```

**Observed response for a minimal device** (G, N, I, J, P, X features):

```
06 01 47 20    → 'G'
06 01 4E 20    → 'N'
06 01 49 20    → 'I'
06 01 4A 20    → 'J'
06 01 50 20    → 'P'
06 01 58 20    → 'X'
06 01 0A 20    → '\n'  (terminator)
```

Feature codes and the conditions under which each is emitted:

| Code  | Feature                                          | Condition                             |
|-------|--------------------------------------------------|---------------------------------------|
| `M`   | LED matrix                                       | MAX7221/HT16K33 matrix enabled        |
| `L`   | I²C LCD                                          | `I2CLCD_enabled == 1`                 |
| `K`   | OLED or Nokia LCD                                | OLED or Nokia LCD count > 0           |
| `G`   | Gear display                                     | Always sent                           |
| `N`   | Named device (`Command_DeviceName` supported)    | Always sent                           |
| `I`   | Unique ID (`Command_UniqueId` supported)         | Always sent                           |
| `J`   | Button count query supported                     | Always sent                           |
| `P`   | Custom protocol supported                        | Always sent                           |
| `X`   | Extended commands supported                      | Always sent                           |
| `R`   | RGB matrix                                       | WS2812B/DM163/Sunfounder matrix > 0   |
| `V`   | Vibration / motor (ShakeIt)                      | Any motor driver enabled              |

The codes are emitted in source order: `M`, `L`, `K`, `G`, `N`, `I`, `J`, `P`, `X`,
optionally `R`, optionally `V`, then `\n`.

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

## 8. Session Lifecycle

Every time SimHub connects to a device it runs a fixed sequence of queries before it
starts sending game data. The sequence has four phases, always in this order:

---

### 8.1 Phase 1 — Hello

SimHub opens the connection by sending a Hello packet using the broadcast Packet ID
(`0xFF`) so it is always accepted regardless of device state.

```
SimHub → 01 01 FF 03  03 31 10  6A      Hello ('1'), broadcast
Device → 03 FF                           ACK
Device → 08 6A                           Single byte: firmware version 'j'
```

SimHub retransmits Hello repeatedly (always with ID=`FF`) until it receives a clean ACK
and version byte. NACKs during this phase (`04 FF 04`, `04 FF 05`, `04 FF 03`) are normal
and occur when a packet is split across receive buffers.

---

### 8.2 Phase 2 — Feature Enumeration

Once Hello succeeds SimHub runs a fixed sequence of queries, each on its own sequential
Packet ID, to discover what the device supports and how many of each peripheral exist.
The exact order observed in captured traces:

| Packet ID | Command           | ARQ data bytes      | Query                     | Device response                                        |
|-----------|-------------------|---------------------|---------------------------|--------------------------------------------------------|
| `00`      | `'0'`             | `03 30`             | Supported features        | Feature chars, one `06 01 XX 20` frame each, then `06 01 0A 20` |
| `01`      | `'4'`             | `03 34`             | RGB LED count             | `08 <count>`                                           |
| `02`      | `'2'`             | `03 32`             | TM1638 module count       | `08 <count>`                                           |
| `03`      | `'B'`             | `03 42`             | Simple 7-seg module count | `08 <count>`                                           |
| `04`      | `'X' "list\n"`    | `03 03 03 58 6C 69 73 74 0A` | Extended command list | String frame per sub-command + `08 0A` terminator |
| `05`      | `'N'`             | `03 4E`             | Device name               | `06 <len> <name> 20` then `06 01 0A 20`               |
| `06`      | `'I'`             | `03 49`             | Unique ID (MAC address)   | `06 <len> <mac> 20` then `06 01 0A 20`                |
| `07`      | `'J'`             | `03 4A`             | Total button count        | `08 <count>`                                           |
| `08`      | `'X' "mcutype\n"` | `03 03 03 58 6D 63 75 74 79 70 65 0A` | MCU signature | `08 1E`, `08 98`, `08 01` (three separate frames) |

If `'V'` (motors) is in the feature string, SimHub also sends `'V' 'C'` to query the motor
count and provider names. Additional `'X'` sub-commands (`tach`, `speedo`, `boost`, `temp`,
`fuel`, `cons`, `encoderscount`) are queried only if the device reported them in the
`X list` response.

**X list response format.** Each supported sub-command is sent as a string-with-newline
frame, then terminated with a single-byte `'\n'`:

```
06 08 6D 63 75 74 79 70 65 0A 20    → "mcutype\n"   (always present)
06 0A 6B 65 65 70 61 6C 69 76 65 0A 20  → "keepalive\n" (always present)
08 0A                                → '\n' end-of-list marker
```

**MCU type response.** Three separate single-byte frames, one per signature byte:

```
08 1E    → SIGNATURE_0 (0x1E)
08 98    → SIGNATURE_1 (0x98)
08 01    → SIGNATURE_2 (0x01)
```

**Device name / Unique ID response.** The value is sent as a single string frame, then a
separate `'\n'` frame:

```
06 <len> <value bytes> 20    → value string
06 01 0A 20                  → '\n' terminator
```

---

### 8.3 Phase 3 — Baud Rate Negotiation

After enumeration SimHub sends the `'8'` command with its preferred baud rate code (see §7).
The baud rate code is packed as a third byte directly in the ARQ payload:

```
ARQ data: 03 38 <code>
          │  │   └─ baud rate code byte (e.g. 0x0B = 11 = 115200)
          │  └─ '8' (0x38)
          └─ MESSAGE_HEADER
```

Observed example (115200 baud, code 11 = `0x0B`):

```
SimHub → 01 01 09 03  03 38 0B  6C      Set baud rate to 115200
Device → 03 09                           ACK
```

The device applies a 200 ms delay, then switches. SimHub follows on its side. All
subsequent traffic runs at the new baud rate.

> If SimHub is satisfied with 19200 (the default) this step is skipped entirely.

---

### 8.4 Phase 4 — Data Streaming

Streaming starts immediately after baud rate negotiation. The first data packet observed
is always `'G'` (gear), with the gear character packed inline:

```
ARQ data: 03 47 20
          │  │  └─ gear char (0x20 = space = no gear selected)
          │  └─ 'G'
          └─ MESSAGE_HEADER
```

SimHub then sends `X keepalive` on every subsequent Packet ID indefinitely as a
connection heartbeat. The device has no handler for `keepalive` — it simply returns,
and the ARQ layer sends back the ACK. This is how SimHub detects connection loss.

Interspersed with keepalives, SimHub sends data update commands whenever game state
changes:

| Command     | Peripheral updated         | Device reply    |
|-------------|----------------------------|-----------------|
| `'6'`       | RGB LEDs                   | `08 15`         |
| `'R'`       | RGB matrix                 | `08 15`         |
| `'P'`       | Custom protocol            | `08 15`         |
| `'3'`       | TM1638 displays + LEDs     | *(none)*        |
| `'S'`       | 7-segment displays         | *(none)*        |
| `'G'`       | Gear indicator             | *(none)*        |
| `'L'`       | I²C LCD                    | *(none)*        |
| `'K'`       | OLED / Nokia LCD           | *(none)*        |
| `'M'`       | LED matrix                 | *(none)*        |
| `'V' 'S'`   | Motors / ShakeIt           | *(none)*        |
| `X keepalive` | *(heartbeat)*            | ACK only        |

During streaming the device may push unsolicited custom packets to SimHub whenever a
button is pressed or an encoder is turned (see §5).

---

## 9. Communication Flow Examples

### 9.1 Hello / Handshake

```
SimHub                                   Device
  |                                         |
  |--[01 01 FF 03  03 31 10  6A]----------->|   ARQ packet ID FF, data: 03 '1' 0x10
  |                                         |
  |<--[03 FF]-------------------------------|   ACK: Packet ID FF accepted
  |<--[08 6A]-------------------------------|   Single-byte: firmware version 'j' (0x6A)
  |                                         |
```

The `0x10` in the ARQ data is consumed and discarded by `Command_Hello` before the
version byte is sent.

### 9.2 Feature Query

```
SimHub                                   Device
  |                                         |
  |--[01 01 00 02  03 30  38]-------------->|   ARQ packet ID 00, data: 03 '0', CRC 38
  |                                         |
  |<--[03 00]-------------------------------|   ACK: Packet ID 00 accepted
  |<--[06 01 47 20]-------------------------|   'G'
  |<--[06 01 4E 20]-------------------------|   'N'
  |<--[06 01 49 20]-------------------------|   'I'
  |<--[06 01 4A 20]-------------------------|   'J'
  |<--[06 01 50 20]-------------------------|   'P'
  |<--[06 01 58 20]-------------------------|   'X'
  |<--[06 01 0A 20]-------------------------|   '\n' terminator
  |                                         |
```

Each feature code is a separate 4-byte string frame. No single combined frame is sent.

### 9.3 RGB LED Count

```
SimHub                                   Device
  |                                         |
  |--[01 01 01 02  03 34  83]-------------->|   ARQ packet ID 01, data: 03 '4', CRC 83
  |                                         |
  |<--[03 01]-------------------------------|   ACK: Packet ID 01 accepted
  |<--[08 00]-------------------------------|   Single-byte: 0 RGB LEDs
  |                                         |
```

### 9.4 RGB LED Data Update

```
SimHub                                   Device
  |                                         |
  |--[ARQ: 03 '6' <RGB data>]-------------->|
  |                                         |
  |<--[03 <ID>]------------------------------|   ACK
  |<--[08 15]--------------------------------|   Single-byte ACK 0x15 (data processed)
  |                                         |
```

### 9.5 Button Press (Device → Host)

```
SimHub                                   Device
  |                                         |
  |                        button pressed   |
  |<--[09 03 02 01 01]----------------------|   Custom packet: button 1 pressed
  |                                         |
```

### 9.6 NACK / Retransmit

The following NACK reason codes have been observed in captured traces:

```
04 FF 04    CRC mismatch on broadcast packet
04 FF 05    Incomplete data on broadcast packet
04 FF 03    Missing CRC byte on broadcast packet
04 01 04    CRC mismatch on packet ID 01
04 01 02    Bad length on packet ID 01
04 01 05    Incomplete data on packet ID 01
```

Example retransmit sequence:

```
SimHub                                   Device
  |                                         |
  |--[01 01 01 02  03 34  FF]-------------->|   Bad CRC (should be 83)
  |                                         |
  |<--[04 00 04]----------------------------|   NACK: last valid=0x00, reason=0x04 (CRC)
  |                                         |
  |--[01 01 01 02  03 34  83]-------------->|   Retransmit with correct CRC
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
