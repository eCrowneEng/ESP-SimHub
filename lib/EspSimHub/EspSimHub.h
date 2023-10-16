#pragma once
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif


// Fake an Arduino Mega
#define SIGNATURE_0 0x1E
#define SIGNATURE_1 0x98
#define SIGNATURE_2 0x01

// A unique identifier for the device.
//  in the future we could use the bytes to generate some
//  other format (ala UUID), but now it's just a unique
//  string tied to the device.
String getUniqueId();