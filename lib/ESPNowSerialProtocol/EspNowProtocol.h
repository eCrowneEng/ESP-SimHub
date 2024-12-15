#pragma once
#include <Arduino.h>

#define MAX_BRIDGE_BYTES 8
#define MAX_SIMHUB_BYTES 32
#define MESSAGE_VERSION 1

typedef struct __attribute__((packed)) EspNowMessage {
  int version;
  int length;
  char simHubBytes[MAX_SIMHUB_BYTES];
  char bridgeBytes[MAX_BRIDGE_BYTES];
} EspNowMessage;


uint8_t* numberToBytes(int number);
int bytesToNumber(unsigned char* bytes);
uint8_t* makeCommand(
  uint8_t* commandPayloadBytes, 
  int commandPayloadBytesCount, 
  uint8_t command, 
  size_t& resultingBytesCount
);

// Protocol is:
// 0x03 <length> <bridgeBytes> 0x0A

#define COMMAND_HEADER 0x03
#define COMMAND_END 0x0A
#define BAUDRATE_COMMAND '8' // matches simhub command
#define PING_COMMAND 'I'
#define PONG_COMMAND 'O'
