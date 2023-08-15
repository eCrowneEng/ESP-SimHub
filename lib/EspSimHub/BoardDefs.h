#pragma once
#include <Arduino.h>

// This is used as a hint for the IDE, but the real values should come from platformio.ini environment
//  as a matter of fact, you need to comment out BOTH if you are compiling for both environments
#define ESP32 // if using ESP8266, comment out
//#define ESP8266 // if using ESP32, comment out


// Here are a bunch of definitions: https://github.com/espressif/arduino-esp32/tree/master/variants
//  each manufacturer calls each pin different
//
// The goal of having this here, is to avoid compilation errors and to centralize a mapping
//  from here, you may correct errors or reconfigure the meaning of these aliases
//
// If a pin name is not here, it doesn't mean you can't use it, you have to figure out how to reference it
// If you picked a board that includes these definitions and you're encountering errors, delete these
// If your the pins are wrong, remap them, or ignore these mappings and use numbers directly, per your boards pinout
//#ifdef ESP32

//#endif
