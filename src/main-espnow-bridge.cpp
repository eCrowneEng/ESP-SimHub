#include <ESP32_NOW_Serial.h>
#include <MacAddress.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <FullLoopbackStream.h>
#include <ESPNowSerialBridge.h>
#include <EspNowProtocol.h>

/**
 * Configure the ESP-NOW bridge
 */

#define DEBUG true
// Where are we reading the SimHub data from?
// Serial = the usb connector available on the board; Serial1 = external USB to Serial adapter connected to the board wires
#define DATA_SERIAL Serial1
#define DATA_UART_RX 17 // RX, or the pin number on the board (17 maybe), connected to TX on the other device
#define DATA_UART_TX 18 // TX, or the pin number on the board (18 maybe), connected to RX on the other device

#if DEBUG
  // If debug is enabled, we will write to this Serial port, use a different one for the data
  // Typically you would want to use Serial for debugging because ESP32 outputs crash data to it
  // But that means that you can't use the "USB" connector in your board, you need an extra USB to Serial adapter, 
  //  or another ESP reading serial from the wire (not through USB)
  #define DEBUG_SERIAL Serial // or Serial1
#else
  #define DEBUG_SERIAL nullptr
#endif

// The device which will receive the data
#define PEER_MAC_ADDRESS {0x34, 0x85, 0x18, 0x95, 0xE4, 0xF0}

#define INITIAL_BAUD_RATE 19200
#define ESPNOW_WIFI_CHANNEL 1
#define ESPNOW_WIFI_MODE WIFI_STA     // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_STA  // WiFi Interface

/**
 * Nothing to configure below here
 */

const MacAddress peer_mac(PEER_MAC_ADDRESS);
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
  DEBUG_SERIAL.begin(115200); // Debug serial at fixed baud rate
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


