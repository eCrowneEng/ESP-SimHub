#pragma once
#include <Arduino.h>

// This is used as a hint for the IDE, but the real values should come from platformio.ini environment
//  as a matter of fact, you need to comment out BOTH if you are compiling for both environments
// #define ESP32 // if using ESP8266, comment out
#define ESP8266 // if using ESP32, comment out

// PIN Aliases no longer here,
//  Use GPIO_NUM_XX instead of D1, D2, etc
