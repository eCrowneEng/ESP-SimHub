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
Set the [preprocessor directive](https://cplusplus.com/doc/tutorial/preprocessor/) called CONNECTION_TYPE to WIFI `#define CONNECTION_TYPE WIFI` (in src/main.cpp). Set DEBUG_BRIDGE to true as well, and set `monitor_speed = 115200` in `platformio.ini`.

## How to connect the ESP to your WiFi
Upload a [WiFi enabled](https://github.com/eCrowneEng/ESP-SimHub/blob/main/src/main.cpp#L4) firmware. Connect USB to computer and open the serial monitor with `115200` baud rate (to catch debug messages). Power the ESP up. 

(If you hardcoded WiFi credentials already, skip this paragraph) It will itself create a new WiFi network (ESP-{a bunch of letters and numbers}) that you can connect to from your phone or computer. Connect to that network and navigate to `http://192.168.4.1`, observe a [captive portal](https://en.wikipedia.org/wiki/Captive_portal). Configure your WiFi and then disconnect.

Once the ESP is connected to a network, it won't create its own. The device should output the IP address it's using, write it down, you'll need it.

If you have access to your router settings, it's worth setting a static ip address or a DHCP reservation to the ESP.. that way you won't have to reconfigure the virtual port every few reconnections.

## How to configure the virtual port
Download [Perle TruePort](https://www.perle.com/downloads/trueport.shtml). Install it. Configure a new COM port. Forward the data to the IP address of your ESP. [Check the manual on the section called "Configuring the COM Port Connection" > "Access Device Server Serial Port"](https://www.perle.com/downloads/drivers/trueport/windows/windows_ug.pdf) Follow the instructions to set it up in LITE mode. .. and that should be it.

## How to configure SimHub to use the virtual port
SimHub should see the virtual port as a normal COM port, and it should be able to "scan it" and do the normal handshake process to query and use it. After confirming everything works, you can upload a new ESP firmware without debug logging.

## ESP32 Support Caveats
Some Arduino APIs for the ESP32 are different, so not everything will work out of the box, but I got the WiFi bridge working and the ShakeIt fans with some effort, as you need to use different includes etc. It's very likely that other features that I haven't tested will need tweaks. But if you're willing to experiment with this, feel free to do so. Checkout this PR https://github.com/eCrowneEng/ESP-SimHub/pull/1 as an example of the things you need to do to achieve compatibility.

### How to enable ESP32
- Open `platformio.ini`, comment out the ESP8266 env and Uncomment the ESP32 env. Also specify your board in the env, by default is set to something like `esp32doit-devkit-v1` because I owned this board, but there are tons of boards for the ESP32, as you can see [here](https://docs.platformio.org/en/latest/boards/index.html#espressif-32). Click on one and it will show you what's its id. 
- Make sure to use the correct pin numbers for the ESP32 as opposed to aliases such as D1, D2, D3.. etc
