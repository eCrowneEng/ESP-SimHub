#pragma once
#include <BoardDefs.h>
#include <BoardWifi.h>

// Fake an Arduino Mega
#define SIGNATURE_0 0x1E
#define SIGNATURE_1 0x98
#define SIGNATURE_2 0x01

// Configure FASTLED with proper pin order
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

// A unique identifier for the device.
//  in the future we could use the bytes to generate some
//  other format (ala UUID), but now it's just a unique
//  string tied to the device.
String getUniqueId();