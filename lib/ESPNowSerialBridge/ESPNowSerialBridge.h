#pragma once

#include <Arduino.h>
#include <FullLoopbackStream.h>
#ifdef ESP32
#include <esp_now.h>
#include <WiFi.h>
#else
#include <espnow.h>
#include <ESP8266WiFi.h>
#pragma message "ESP8266 ESP-NOW is very experimental, use at your own risk"
#endif
#include <EspNowProtocol.h>



class ESPNowSerialBridge
{
public:
  uint8_t peerMac[6];
  uint8_t channel;
  EspNowMessage outgoingMessage;
  EspNowMessage incomingMessage;
  EspNowMessage bridgeMessage;
#ifdef ESP32
  esp_now_peer_info_t peerInfo;
#endif

  ESPNowSerialBridge(const uint8_t* peerMac, uint8_t channel) {
    memcpy(this->peerMac, peerMac, 6);
    this->channel = channel;
  }

  void begin(unsigned long baud) {
    if (baud == 0 || baud == 19200) {
#if DEBUG_BRIDGE
      Serial.println("default baud rate, no change");
#endif
      return;
    }

    // Clear any existing SimHub bytes
    memset(bridgeMessage.bridgeBytes, 0, bridgeMessage.length);

    // Convert baud rate to bytes and create baud command
    auto baudBytes = numberToBytes(baud);
    auto length = 3;

    size_t commandSize;
    auto command = makeCommand(baudBytes, length, BAUDRATE_COMMAND, commandSize);

    memcpy(bridgeMessage.bridgeBytes, command, commandSize);
    bridgeMessage.length = commandSize;

#if DEBUG_BRIDGE
    Serial.print(">> Bridge message bytes (byte code): ");
    for (int i = 0; i < bridgeMessage.length; i++) {
        Serial.printf("%02X ", bridgeMessage.bridgeBytes[i]);
      }
    Serial.println();
#endif
    this->flushMessage(bridgeMessage);
  }

  void setup(FullLoopbackStream *outgoingStream, FullLoopbackStream *incomingStream) {
    Serial.println("Setting up ESPNowSerialBridge");

    // Store the instance pointer in the static member
    instance = this;

    this->outgoingStream = outgoingStream;
    this->incomingStream = incomingStream;


     // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi mode set to STA");

    // Init ESP-NOW
#ifdef ESP32
    if (esp_now_init() != ESP_OK) {
#else
    if (esp_now_init() != 0) {
#endif
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    Serial.println("ESP-NOW initialized");

#ifdef ESP32
    // Register peer
    memcpy(peerInfo.peer_addr, this->peerMac, 6);
    peerInfo.channel = this->channel;
    peerInfo.encrypt = false;
#else
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
#endif

    esp_now_register_send_cb(staticOnDataSent);
    esp_now_register_recv_cb(staticHandleData);

    // Register peer
#ifdef ESP32
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
#else
    if (esp_now_add_peer(this->peerMac, ESP_NOW_ROLE_COMBO, this->channel, NULL, 0) != 0) {
#endif
      Serial.println("Failed to add peer");
      return;
    }
    Serial.println("Peer registered");


    Serial.print("This device's MAC address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPmacAddress() : WiFi.macAddress());
  }
  
  void loop() {
    this->flush();
  }

  void flush() {
    // if there is data available in the wifi stream, it's meant
    //  to go from Serial port to TCP client
    size_t availableLength = this->outgoingStream->available();
    if (availableLength)
    {

      int bytesRead = this->outgoingStream->readBytes(outgoingMessage.simHubBytes, availableLength);
      outgoingMessage.version = MESSAGE_VERSION;
      outgoingMessage.length = bytesRead;
      memset(outgoingMessage.bridgeBytes, 0, sizeof(outgoingMessage.bridgeBytes));
      this->outgoingStream->clear();

#if DEBUG_BRIDGE
      Serial.print(">> Response bytes (byte code): ");
      for (int i = 0; i < bytesRead; i++) {
        Serial.printf("%02X ", outgoingMessage.simHubBytes[i]);
      }
      Serial.println();
#endif
      this->flushMessage(outgoingMessage);
    }
  }

  void flushMessage(EspNowMessage& message) {
      // Copy message struct to buffer
      uint8_t buffer[sizeof(EspNowMessage)];
      memcpy(buffer, &message, sizeof(EspNowMessage));
      // Send buffer to ESP-NOW
#ifdef ESP32
      esp_err_t result = esp_now_send(peerInfo.peer_addr, buffer, sizeof(EspNowMessage));
      if (result != ESP_OK) {
        const char* error_msg = esp_err_to_name(result);
        Serial.printf(">> Error with response: %s (0x%x)\n", error_msg, result);
        
        // Common error handling
        switch(result) {
          case ESP_ERR_ESPNOW_NOT_INIT:
            Serial.println(">> ESP-NOW not initialized");
            break;
          case ESP_ERR_ESPNOW_ARG:
            Serial.println(">> Invalid argument");
            break;
          case ESP_ERR_ESPNOW_NO_MEM:
            Serial.println(">> Out of memory");
            break;
          case ESP_ERR_ESPNOW_FULL:
            Serial.println(">> Peer list is full");
            break;
          case ESP_ERR_ESPNOW_NOT_FOUND:
            Serial.println(">> Peer not found");
            break;
          case ESP_ERR_ESPNOW_INTERNAL:
            Serial.println(">> Internal error");
            break;
          case ESP_ERR_ESPNOW_EXIST:
            Serial.println(">> Peer already exists");
            break;
          case ESP_ERR_ESPNOW_IF:
            Serial.println(">> Interface error");
            break;
        }
      }
#else
      int result = esp_now_send(this->peerMac, buffer, sizeof(EspNowMessage));
      if (result != 0) {
        Serial.printf(">> Error with response: %d\n", result);
      }
#endif
  }

// Callback when data is sent
#ifdef ESP32  
  void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
#else
  void onDataSent(const uint8_t *mac_addr, uint8_t status) {
    if (status != 0) {
#endif
#if DEBUG_BRIDGE
      Serial.print("\n>> Last Packet Send Status: ");
#ifdef ESP32
      Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#else
      Serial.println(status == 0 ? "Success" : "Fail");
#endif
#endif
    }
  }

  // Callback when data is received
#ifdef ESP32  
//  void handleData(const esp_now_recv_info_t *peerInfo, const uint8_t *data, int len) {
  void handleData(const uint8_t *peerInfo, const uint8_t *data, uint8_t len) {
#else
  void handleData(const uint8_t *peerInfo, const uint8_t *data, uint8_t len) {
#endif
    byte buffer[sizeof(EspNowMessage)];
    memcpy(&incomingMessage, data, sizeof(EspNowMessage));    
    if (incomingMessage.version != MESSAGE_VERSION) {
#if DEBUG_BRIDGE
      Serial.println("<< Mismatching version for message, it could be corrupted or bridge device is outdated");
#endif
      this->incomingStream->clear();
      return;
    }

#if DEBUG_BRIDGE
    Serial.print("<< Request received (byte code): ");
    if (incomingMessage.length > 0 && incomingMessage.bridgeBytes[0] == COMMAND_HEADER) {
      for (int i = 0; i < incomingMessage.length; i++) {
        Serial.printf("%02X ", incomingMessage.bridgeBytes[i]);
      }
    } else {
      for (int i = 0; i < incomingMessage.length; i++) {
        Serial.printf("%02X ", incomingMessage.simHubBytes[i]);
      }
    }
    Serial.println();
#endif
    this->incomingStream->clear();

    if (incomingMessage.length > 0 && incomingMessage.bridgeBytes[0] == COMMAND_HEADER) {
      // treat it as bridge bytes
      this->handleBridgeMessage(incomingMessage);
    } else {
      // treat it as serial bytes
      this->incomingStream->write(incomingMessage.simHubBytes, incomingMessage.length);
    }
  }

  void handleBridgeMessage(EspNowMessage& message) {
    uint8_t length = message.bridgeBytes[1];
    uint8_t command = message.bridgeBytes[2];

    if (message.bridgeBytes[2 + length + 1] != COMMAND_END) {
      // invalid command / length
      return;
    }

    switch (command) {
      case PING_COMMAND: {
  #if DEBUG_BRIDGE
        Serial.println(">> PING received");
  #endif
        bridgeMessage.version = MESSAGE_VERSION;
        size_t resultingByteCount = 0;
        uint8_t* pingBytes = makeCommand(nullptr, 0, PONG_COMMAND, resultingByteCount);
        bridgeMessage.length = resultingByteCount;
        memcpy(bridgeMessage.bridgeBytes, pingBytes, resultingByteCount);
        // print the bytes sent to the bridge
        Serial.print(">> PONG bytes (byte code): ");
        for (int i = 0; i < bridgeMessage.length; i++) {
          Serial.printf("%02X ", bridgeMessage.bridgeBytes[i]);
        }
        Serial.println();
        this->flushMessage(bridgeMessage);
        break;
      }
      default:
        break;
    }
  }


  // Static callback function that ESP-NOW can work with
#ifdef ESP32  
//  static void staticHandleData(const esp_now_recv_info_t *peerInfo, const uint8_t *data, int len) {
  static void staticHandleData(const uint8_t *peerInfo, const uint8_t *data, int len) {
#else
  static void staticHandleData(uint8_t *peerInfo, uint8_t *data, uint8_t len) {
#endif
    // Forward to instance method if instance exists
    if (ESPNowSerialBridge::instance) {
      ESPNowSerialBridge::instance->handleData(peerInfo, data, len);
    }
  }

  // Static callback function for send status
#ifdef ESP32  
  static void staticOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#else
  static void staticOnDataSent(uint8_t *mac_addr, uint8_t status) {
#endif
    // Forward to instance method if instance exists
    if (ESPNowSerialBridge::instance) {
      ESPNowSerialBridge::instance->onDataSent(mac_addr, status);
    }
  }

  // Static member to hold instance pointer
  static ESPNowSerialBridge* instance;
  FullLoopbackStream *incomingStream;
  FullLoopbackStream *outgoingStream;
};
