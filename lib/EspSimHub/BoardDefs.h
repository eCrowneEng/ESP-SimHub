#pragma once
#include <Arduino.h>

// Here are a bunch of definitions: https://github.com/espressif/arduino-esp32/tree/master/variants
//  each manufacturer calls each pin different
// 
// The goal of having this here, is to avoid compilation errors and to centralize a mapping
//  from here, you may correct errors or reconfigure the meaning of these aliases
//
// If a pin name is not here, it doesn't mean you can't use it, you have to figure out how to reference it
// If you picked a board that includes these definitions and you're encountering errors, delete these
// If your the pins are wrong, remap them, or ignore these mappings and use numbers directly, per your boards pinout
#ifdef ESP32
static const uint D2 = 22;
static const uint D3 = 12;
static const uint D4 = 24;
static const uint D5 = 34;
static const uint D6 = 13;
static const uint D7 = 14;
static const uint D8 = 15;
static const uint D9 = 16;
static const uint D10 = 17;
static const uint D11 = 35; 
static const uint D12 = 18;
static const uint D13 = 20;
static const uint D14 = 25;
static const uint D15 = 21;
static const uint D16 = 27;
static const uint D17 = 38;
static const uint D18 = 42;
static const uint D19 = 40;
static const uint D20 = 41;
static const uint D21 = 39;
static const uint D22 = 36;
#endif
