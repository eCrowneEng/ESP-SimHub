# SimHub firmware for ESP32 and ESP8266
Documentation: [ESP SimHub Firmware documentation - ecrowne.com](http://ecrowne.com/)

Head over to the discord server to discuss this firmware: [DISCORD](https://discord.gg/CfvJySWTcE) 

## What is this?
Firmware source code for ESP32 and ESP8266 that is compatible with SimHub. It's derived from what SimHub lets you setup, but tweaked with a thin compatibility layer for the ESP32 and ESP8266.

## How To
[Getting started with ESP-SimHub for ESP32 and ESP8266](http://ecrowne.com/esp-simhub/getting-started)

- Download this code
- Install [Git](https://git-scm.com/downloads)
- Install [VSCode](https://code.visualstudio.com/Download) and [Platformio](https://platformio.org/platformio-ide)
- Open with VSCode
- Select the environment you want to use, either ESP8266 or ESP32 from the dropdown in the platformio bottom bar
- Tweak src/main.cpp to your needs
- Upload it to your device

## How does the SimHub Wireless connection work?
SimHub assumes there is a serial port that it can talk to, uses a certain protocol that accounts for errors [Automatic Repeat Request](https://en.wikipedia.org/wiki/Automatic_repeat_request).

Instead of having the feature device connected to the computer directly, we use "bridges" that forward the data to the feature device and back to the computer.

- For WiFi we create a "virtual com port" using Perle TruePort or similar software which will forward all communication to a certain IP and the feature device is connected to that IP.

- For ESP-Now we use a 2 ESPs, one with the features and one connected to the computer where SimHub is running. One device just forwards the data to the feature device and back to the computer, while the other device receives and consumes the data.

For both protocols, on the Feature device side, we modify the code to create a the appropriate listener receiving data (either from WiFi or ESP-Now), we put the received data in a buffer (similar to the serial port) and we allow the modified Arduino SimHub client to consume it from the buffer, as if it was a normal seial connection.

## How to configure the firmware to use ESPNow
Set the [preprocessor directive](https://cplusplus.com/doc/tutorial/preprocessor/) called CONNECTION_TYPE to ESPNOW `#define CONNECTION_TYPE ESP_NOW` (in src/main.cpp). Set DEBUG_BRIDGE to true as well, and set `monitor_speed = 115200` in `platformio.ini`. Configure the MAC address of the ESP that will remain connected to the computer (not this one) in `#define ESPNOW_PEER_MAC {0x34, 0x85, 0x18, 0x90, 0x7A, 0x00}` (in src/main.cpp) for a device with MAC address `34:85:18:90:7A:00`. Upload the ESP-SimHub firmware to the ESP with the features.

Switch the environment to `env:espnow-bridge`, configure the code in `src/main-espnow.cpp`, set the MAC address of the ESP that has all the features (not this one) in `#define ESPNOW_PEER_MAC {0x34, 0x85, 0x18, 0x90, 0x7A, 0x01}` (in src/main-espnow.cpp) for a device with MAC address `34:85:18:90:7A:01`, and upload it to the ESP that will remain connected to the computer.

## How to configure the firmware to use WiFi
[Using WiFi with SimHub and ESP32, ESP8266](http://ecrowne.com/esp-simhub/wifi)

## How to switch environments for ESP8266 or ESP32 and configure boards
[How to configure ESP32 and ESP8266 in Platformio, and how to switch between them](http://ecrowne.com/esp-simhub/board-configuration)
