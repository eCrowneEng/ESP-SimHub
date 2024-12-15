#ifdef ESP32
#include <ESP32_NOW_Serial.h>
#include <MacAddress.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <FullLoopbackStream.h>
#include <EspNowProtocol.h>

/**
 * Configure the ESP-NOW bridge
 * 
 * This device will be connected to the computer running SimHub, and will forward the data to the feature device and back to the computer.
 * 
 * This device's peer is the feature device. It needs to know its MAC address to be able to send data to it.
 * The feature device's peer is this device.
 * 
 * In order to see the MAC address of this device, upload the code below with `#define DEBUG true` and `#define DEBUG_SERIAL Serial` to your ESP32 and 
 *  open the serial monitor.
 * If you're using an ESP8266, use the code at the very bottom of this file to print the MAC address.
 * 
 * Please notice that this device uses Serial communication. So using `#define DEBUG true` and outputting to Serial will break connecting to SimHub.
 *  IF YOU NEED TO DEBUG WHILE ACTIVE, USE A DIFFERENT SERIAL PORT FOR THE DATA AND FOR DEBUGGING, which means you need to use a USB to Serial 
 *  adapter or another ESP reading those pins. I recommend using the device's USB/Serial port for debugging and Serial1 for the data, because
 *  when the device crashes, it outputs crash data to the Serial port, and you need to be able to read it.
 */

#define DEBUG false
// Where are we reading the SimHub data from?
// Serial = the usb connector available on the board; 
// Serial1 = external USB to Serial adapter or separate ESP device connected to the board wires
#define DATA_SERIAL Serial // Serial for the USB connector on the board; Serial1 for external serial communication
#define DATA_UART_RX RX // RX for USB/Serial, or 17 or some other pin number on the board connected to TX on the other device for external serial communication
#define DATA_UART_TX TX // TX for USB/Serial, or 18 or some other pin number on the board connected to RX on the other device for external serial communication

#if DEBUG
  // If debug is enabled, we will write to this Serial port
  // If BOTH data and debug are using the same Serial port, you won't be able to connect to SimHub, but you can still see the MAC address
  #define DEBUG_SERIAL Serial1 // or Serial or Serial1
  #define DEBUG_UART_RX 17 // or RX or 17
  #define DEBUG_UART_TX 18 // or TX or 18
#endif

// The device which will receive the data
#define ESPNOW_PEER_MAC {0x34, 0x85, 0x18, 0x95, 0xE4, 0xF0}


/**
 * It's unlikely that you need to change anything below here, they're here in case you need to tweak them.
 */
#define INITIAL_BAUD_RATE 19200       // 19200 is the default baud rate for SimHub
#define ESPNOW_WIFI_CHANNEL 1         // You may need to change this, and if you do, update the channel in src/
#define ESPNOW_WIFI_MODE WIFI_STA     // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_STA  // WiFi Interface


/**
 * Nothing to configure below here
 * 
 * Feel free to read through the code and tweak if you need regardless.
 */


const MacAddress peer_mac(ESPNOW_PEER_MAC);
ESP_NOW_Serial_Class NowSerial(peer_mac, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IF);
EspNowMessage incomingMessage;
EspNowMessage outgoingMessage;
EspNowMessage bridgeMessage;
bool connected = false;
unsigned long lastMessageTimeMillis = 0;
unsigned long lastPingTimeMillis = 0;
unsigned long maxTimeBetweenMessagesMillis = 5000;
unsigned long maxTimeBetweenPingsMillis = 1000;


void setup() {
#if DEBUG
if (DEBUG_SERIAL != Serial) {
  DEBUG_SERIAL.begin(115200, SERIAL_8N1, DEBUG_UART_RX, DEBUG_UART_TX);
}
#endif
  DATA_SERIAL.begin(INITIAL_BAUD_RATE, SERIAL_8N1, DATA_UART_RX, DATA_UART_TX);

#if DEBUG
  DEBUG_SERIAL.print("WiFi Mode: ");
  DEBUG_SERIAL.println(ESPNOW_WIFI_MODE == WIFI_AP ? "AP" : "Station");
#endif
  WiFi.mode(ESPNOW_WIFI_MODE);

#if DEBUG
  DEBUG_SERIAL.print("Channel: ");
  DEBUG_SERIAL.println(ESPNOW_WIFI_CHANNEL);
#endif
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  while (!(WiFi.STA.started() || WiFi.AP.started())) {
    delay(100);
  }

#if DEBUG
  DEBUG_SERIAL.print("MAC Address: ");
  DEBUG_SERIAL.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPmacAddress() : WiFi.macAddress());
  DEBUG_SERIAL.print("Peer MAC Address: ");
  DEBUG_SERIAL.println(peer_mac.toString());

  // Start the ESP-NOW communication
  DEBUG_SERIAL.println("ESP-NOW communication starting...");
#endif
  NowSerial.begin();

#if DEBUG
  DEBUG_SERIAL.println("You can now send data to the peer device using the Serial Monitor.\n");
#endif
}

void handleBridgeMessage(EspNowMessage& message) {
  uint8_t length = message.bridgeBytes[1];
  uint8_t command = message.bridgeBytes[2];

  if (message.bridgeBytes[2 + length + 1] != COMMAND_END) {
    // invalid command / length
    return;
  }

  switch (command) {
    case BAUDRATE_COMMAND: {
      if (length != 3) {
        // Baudrate should always be 3 bytes
        return;
      }
      int baud = bytesToNumber((unsigned char*)message.bridgeBytes + 3);
      DATA_SERIAL.begin(baud, SERIAL_8N1, DATA_UART_RX, DATA_UART_TX);
      break;
    }
    case PONG_COMMAND: {
      connected = true;
      break;
    }
      
    default:
      // unknown command
      break;
  }

  // reset the message length to 0 so that it's not processed again
  message.length = 0;
}


void flushMessage(EspNowMessage& message) {
  uint8_t buffer[sizeof(EspNowMessage)];
  memcpy(buffer, &message, sizeof(EspNowMessage));
  NowSerial.write(buffer, sizeof(EspNowMessage));
  NowSerial.flush();
}

void loop() {
  // Read from ESP-NOW and write to DATA_SERIAL
  if (NowSerial.available()) {
    lastMessageTimeMillis = millis();
    byte buffer[sizeof(EspNowMessage)];
    NowSerial.readBytes(buffer, sizeof(EspNowMessage));
    memcpy(&incomingMessage, buffer, sizeof(EspNowMessage));
    
#if DEBUG
    DEBUG_SERIAL.print("Received message, length: ");
    DEBUG_SERIAL.print(incomingMessage.length);
    DEBUG_SERIAL.print(", is bridge command: ");
    DEBUG_SERIAL.println(incomingMessage.bridgeBytes[0] == COMMAND_HEADER);
#endif

    if (incomingMessage.length > 0 && incomingMessage.bridgeBytes[0] == COMMAND_HEADER) {
      handleBridgeMessage(incomingMessage);
#if DEBUG
      DEBUG_SERIAL.println("Processed bridge command");
#endif
    } else {
      DATA_SERIAL.write(incomingMessage.simHubBytes, incomingMessage.length);
#if DEBUG
      DEBUG_SERIAL.print("Wrote to DataSerial: ");
      DEBUG_SERIAL.print(incomingMessage.length);
      DEBUG_SERIAL.println(" bytes");
#endif
    }
  }

  // Read from DATA_SERIAL and write to ESP-NOW
  if (DATA_SERIAL.available() && NowSerial.availableForWrite() && connected) {
#if DEBUG
    DEBUG_SERIAL.print("DataSerial available bytes: ");
    DEBUG_SERIAL.println(DATA_SERIAL.available());
#endif

    int availableBytes = min(DATA_SERIAL.available(), MAX_SIMHUB_BYTES);
    outgoingMessage.version = MESSAGE_VERSION;
    outgoingMessage.length = DATA_SERIAL.readBytes(outgoingMessage.simHubBytes, availableBytes);
    flushMessage(outgoingMessage);

#if DEBUG
    DEBUG_SERIAL.print("Sent message, length: ");
    DEBUG_SERIAL.println(outgoingMessage.length);
#endif
  } else if (!connected) {
#if DEBUG
    if (DATA_SERIAL.available()) {
      Serial.println("Dropping data - not connected");
    }
#endif
    while (DATA_SERIAL.available()) {
      DATA_SERIAL.read();
    }
  }
  
  // if no messages for a while mark as disconnected and send ping
  if (millis() - lastMessageTimeMillis > maxTimeBetweenMessagesMillis && millis() - lastPingTimeMillis > maxTimeBetweenPingsMillis) {
#if DEBUG
    DEBUG_SERIAL.println("Connection timeout - sending ping");
    DEBUG_SERIAL.print("Time since last message: ");
    DEBUG_SERIAL.print(millis() - lastMessageTimeMillis);
    DEBUG_SERIAL.println("ms");
#endif

    connected = false;
    DATA_SERIAL.begin(INITIAL_BAUD_RATE, SERIAL_8N1, DATA_UART_RX, DATA_UART_TX);

    bridgeMessage.version = MESSAGE_VERSION;
    size_t resultingByteCount = 0;
    uint8_t* pingBytes = makeCommand(nullptr, 0, PING_COMMAND, resultingByteCount);
    bridgeMessage.length = resultingByteCount;
    memset(bridgeMessage.simHubBytes, 0, sizeof(outgoingMessage.simHubBytes));
    memcpy(bridgeMessage.bridgeBytes, pingBytes, resultingByteCount);
    flushMessage(bridgeMessage);
    lastPingTimeMillis = millis();
  }
}
#else
#pragma message "ESP8266 for the bridge device is not supported"

/**
 * This code is here for reference, it's not used by the bridge device
 *  Upload it to your ESP8266 to see the MAC address
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 MAC address:");
}

void loop() {
    Serial.println(WiFi.macAddress());
    delay(500);
}
#endif