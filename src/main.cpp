#include <Arduino.h>
#include <EspSimHub.h>

// No longer have to define whether it's an ESP32 or ESP8266, just do an initial compilation and
//  VSCode will pick  up the right environment from platformio.ini
 
#if ESP32
// certain features are gated for these boards, so make sure to set them if you're using them
#define USING_ESP32_C3 false
#define USING_ESP32_S2 false
#define USING_ESP32_S3 false
#endif


#define INCLUDE_WIFI true
// Less secure if you plan to commit or share your files, but saves a bunch of memory. 
//  If you hardcode credentials the device will only work in your network
#define USE_HARDCODED_CREDENTIALS false

#if INCLUDE_WIFI
#if USE_HARDCODED_CREDENTIALS
#define WIFI_SSID "Wifi NAME"
#define WIFI_PASSWORD "WiFi Password"
#endif

#define BRIDGE_PORT 10001 // Perle TruePort uses port 10,001 for the first serial routed to the client
#define DEBUG_TCP_BRIDGE false // emits extra events to Serial that show network communication, set to false to save memory and make faster

#include <TcpSerialBridge2.h>
#include <ECrowneWifi.h>
#include <FullLoopbackStream.h>

FullLoopbackStream outgoingStream;
FullLoopbackStream incomingStream;

#endif // INCLUDE_WIFI

#define DEVICE_NAME "HelloWorldEsp" //{"Group":"General","Name":"DEVICE_NAME","Title":"Device name,\r\n make sure to use a unique name when using multiple arduinos","DefaultValue":"SimHub Dash","Type":"string","Template":"#define DEVICE_NAME \"{0}\""}

// Known working features:
//  
// #define INCLUDE_RGB_LEDS_NEOPIXELBUS        // use this instead of INCLUDE_WS2812B
//#define INCLUDE_RGB_MATRIX_NEOPIXELBUS      // use this instead of INCLUDE_WS2812B_MATRIX
//#define INCLUDE_WS2812B                     // consider using INCLUDE_RGB_LEDS_NEOPIXELBUS {"Name":"INCLUDE_WS2812B","Type":"autodefine","Condition":"[WS2812B_RGBLEDCOUNT]>0"}
//#define INCLUDE_WS2812B_MATRIX              // consider using INCLUDE_WS2812B_MATRIX		 {"Name":"INCLUDE_WS2812B_MATRIX","Type":"autodefine","Condition":"[WS2812B_MATRIX_ENABLED]>0"}
//#define INCLUDE_BUTTONS                     //{"Name":"INCLUDE_BUTTONS","Type":"autodefine","Condition":"[ENABLED_BUTTONS_COUNT]>0","IsInput":true}
//#define INCLUDE_BUTTONMATRIX                //{"Name":"INCLUDE_BUTTONMATRIX","Type":"autodefine","Condition":"[ENABLED_BUTTONMATRIX]>0","IsInput":true}
//#define INCLUDE_TM1637                      //{"Name":"INCLUDE_TM1637","Type":"autodefine","Condition":"[TM1637_ENABLEDMODULES]>0"}
//#define INCLUDE_TM1638                      //{"Name":"INCLUDE_TM1638","Type":"autodefine","Condition":"[TM1638_ENABLEDMODULES]>0"}
//#define INCLUDE_OLED                        //{"Name":"INCLUDE_OLED","Type":"autodefine","Condition":"[ENABLED_OLEDLCD]>0"}
//#define INCLUDE_I2CLCD                      //{"Name":"INCLUDE_I2CLCD","Type":"autodefine","Condition":"[I2CLCD_enabled]>0"}
//#define INCLUDE_MAX7221MATRIX               //{"Name":"INCLUDE_MAX7221MATRIX","Type":"autodefine","Condition":"[MAX7221_MATRIX_ENABLED]>0"}
//#define INCLUDE_MAX7221_MODULES             //{"Name":"INCLUDE_MAX7221_MODULES","Type":"autodefine","Condition":"[MAX7221_ENABLEDMODULES]>0"}
//#define INCLUDE_SHAKEITPWMFANS              //{"Name":"INCLUDE_SHAKEITPWMFANS","Type":"autodefine","Condition":"[SHAKEITPWMFANS_ENABLED_MOTORS]>0"}

// Untested features, please open a GitHub Issue, or post in the discord if you try them
//
//#define INCLUDE_WS2801                      //{"Name":"INCLUDE_WS2801","Type":"autodefine","Condition":"[WS2801_RGBLEDCOUNT]>0"}
//#define INCLUDE_PL9823                      //{"Name":"INCLUDE_PL9823","Type":"autodefine","Condition":"[PL9823_RGBLEDCOUNT]>0"}
//#define INCLUDE_LEDBACKPACK                 //{"Name":"INCLUDE_LEDBACKPACK","Type":"autodefine","Condition":"[ENABLE_ADA_HT16K33_7SEGMENTS]>0 || [ENABLE_ADA_HT16K33_BiColorMatrix]>0"}
//#define INCLUDE_TM1637_6D                   //{"Name":"INCLUDE_TM1637_6D","Type":"autodefine","Condition":"[TM1637_6D_ENABLEDMODULES]>0"}
//#define INCLUDE_NOKIALCD                    //{"Name":"INCLUDE_NOKIALCD","Type":"autodefine","Condition":"[ENABLED_NOKIALCD]>0"}
//#define INCLUDE_HT16K33_SINGLECOLORMATRIX   //{"Name":"INCLUDE_HT16K33_SINGLECOLORMATRIX","Type":"autodefine","Condition":"[ENABLE_ADA_HT16K33_SingleColorMatrix]>0"}
//#define INCLUDE_74HC595_GEAR_DISPLAY        //{"Name":"INCLUDE_74HC595_GEAR_DISPLAY","Type":"autodefine","Condition":"[ENABLE_74HC595_GEAR_DISPLAY]>0"}
//#define INCLUDE_6c595_GEAR_DISPLAY          //{"Name":"INCLUDE_6c595_GEAR_DISPLAY ","Type":"autodefine","Condition":"[ENABLE_6C595_GEAR_DISPLAY]>0"}
//#define INCLUDE_TACHOMETER                  //{"Name":"INCLUDE_TACHOMETER ","Type":"autodefine","Condition":"[ENABLE_TACHOMETER]>0"}
//#define INCLUDE_BOOSTGAUGE                  //{"Name":"INCLUDE_BOOSTGAUGE ","Type":"autodefine","Condition":"[ENABLE_BOOSTGAUGE]>0"}
//#define INCLUDE_SPEEDOGAUGE                 //{"Name":"INCLUDE_SPEEDOGAUGE","Type":"autodefine","Condition":"[ENABLE_SPEEDOGAUGE]>0"}
//#define INCLUDE_FUELGAUGE                   //{"Name":"INCLUDE_FUELGAUGE ","Type":"autodefine","Condition":"[ENABLE_FUELGAUGE]>0"}
//#define INCLUDE_TEMPGAUGE                   //{"Name":"INCLUDE_TEMPGAUGE ","Type":"autodefine","Condition":"[ENABLE_TEMPGAUGE]>0"}
//#define INCLUDE_CONSGAUGE                   //{"Name":"INCLUDE_CONSGAUGE ","Type":"autodefine","Condition":"[ENABLE_CONSGAUGE]>0"}
//#define INCLUDE_SHAKEITADASHIELD            //{"Name":"INCLUDE_SHAKEITADASHIELD ","Type":"autodefine","Condition":"[ADAMOTORS_SHIELDSCOUNT]>0"}
//#define INCLUDE_SHAKEITDKSHIELD             //{"Name":"INCLUDE_SHAKEITDKSHIELD ","Type":"autodefine","Condition":"[DKMOTOR_SHIELDSCOUNT]>0"}
//#define INCLUDE_SHAKEITL298N                //{"Name":"INCLUDE_SHAKEITL298N","Type":"autodefine","Condition":"[L98NMOTORS_ENABLED]>0"}
//#define INCLUDE_SHAKEITMOTOMONSTER          //{"Name":"INCLUDE_SHAKEITMOTOMONSTER","Type":"autodefine","Condition":"[MOTOMONSTER_ENABLED]>0"}
//#define INCLUDE_SHAKEIDUALVNH5019           //{"Name":"INCLUDE_SHAKEIDUALVNH5019","Type":"autodefine","Condition":"[DUALVNH5019_ENABLED]>0"}
//#define INCLUDE_SHAKEITPWM                  //{"Name":"INCLUDE_SHAKEITPWM","Type":"autodefine","Condition":"[SHAKEITPWM_ENABLED_MOTORS]>0"}
//#define INCLUDE_ENCODERS                    //{"Name":"INCLUDE_ENCODERS","Type":"autodefine","Condition":"[ENABLED_ENCODERS_COUNT]>0","IsInput":true}
//#define INCLUDE_DM163_MATRIX                //{"Name":"INCLUDE_DM163_MATRIX","Type":"autodefine","Condition":"[DM163_MATRIX_ENABLED]>0"}
//#define INCLUDE_SUNFOUNDERSH104P_MATRIX     //{"Name":"INCLUDE_SUNFOUNDERSH104P_MATRIX","Type":"autodefine","Condition":"[SUNFOUNDERSH104P_MATRIX_ENABLED]>0"}


// Gamepad support =======================
// Untested, it may work with ESP32 S3 and newer devices that support USB HID
//  If you want to test, remove this "if defined ..."
#if defined (__AVR_ATmega32U4__)
//#define INCLUDE_GAMEPAD                     //{"Name":"INCLUDE_GAMEPAD","Type":"autodefine","Condition":"[ENABLE_MICRO_GAMEPAD]>0"}
#endif
//#define INCLUDE_GAMEPADAXIS                 //{"Name":"INCLUDE_GAMEPADAXIS","Type":"autodefine","Condition":"[GAMEPAD_AXIS_01_ENABLED]>0 || [GAMEPAD_AXIS_02_ENABLED]>0 || [GAMEPAD_AXIS_03_ENABLED]>0"}

#ifdef INCLUDE_GAMEPADAXIS
#ifndef  INCLUDE_GAMEPAD
#error Gamepad option must be enabled in order to use analog axis.
#endif // ! INCLUDE_GAMEPAD
#endif
// END Gamepad support ===================






#if (defined(__AVR__))
#include <avr\pgmspace.h>
#else
#include <pgmspace.h>
#endif

#include <EEPROM.h>
#include <SPI.h>
#include "Arduino.h"
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "FlowSerialRead.h"
#include "setPwmFrequency.h"
#include "SHDebouncer.h"
#include "SHButton.h"

// ----------------------------------------------------- HW SETTINGS, PLEASE REVIEW ALL -------------------------------------------

#define ENABLE_MICRO_GAMEPAD 0           //{"Group":"GAMEPAD","Name":"ENABLE_MICRO_GAMEPAD","Title":"Enable arduino micro gamepad output for all the activated buttons/encoders","DefaultValue":"0","Type":"bool"}
#define MICRO_GAMEPAD_ENCODERPRESSTIME 50 //{"Name":"MICRO_GAMEPAD_ENCODERPRESSTIME","Title":"Define how long (in milliseconds) the encoder related button will be hold after an encoder movement","DefaultValue":"50","Type":"int","Condition":"ENABLE_MICRO_GAMEPAD>0","Max":100}

// -------------------------------------------------------------------------------------------------------
// TM1638 Modules ----------------------------------------------------------------------------------------
// http://www.dx.com/p/jy-mcu-8x-green-light-digital-tube-8x-key-8x-double-color-led-module-104329
// https://www.fasttech.com/products/0/10006503/1497600-jy-mcu-8x-digital-tube-red-led-module
// -------------------------------------------------------------------------------------------------------
// Number of Connected TM1638/1639 modules
// 0 disabled, > 0 enabled
#define TM1638_ENABLEDMODULES 0 //{"Group":"TM1638 7 Segments Modules","Name":"TM1638_ENABLEDMODULES","Title":"TM1638 modules connected","DefaultValue":"0","Type":"int","Max":6}
#ifdef INCLUDE_TM1638
#define TM1638_SWAPLEDCOLORS 0  //{"Name":"TM1638_SWAPLEDCOLORS","Title":"Reverse Red and green colors ","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 0"}
#define TM1638_DIO 25  			//{"Name":"TM1638_DIO","Title":"Common data (DIO) digital pin number","DefaultValue":"8","Type":"pin;TM1638 DIO","Condition":"TM1638_ENABLEDMODULES > 0"}
#define TM1638_CLK 26  			//{"Name":"TM1638_CLK","Title":"Common clock (CLK) digital pin number","DefaultValue":"7","Type":"pin;TM1638 CLK","Condition":"TM1638_ENABLEDMODULES > 0"}
#define TM1638_STB1 27 			//{"Name":"TM1638_STB1","Title":"1st module strobe (STB0) digital pin number","DefaultValue":"9","Type":"pin;TM1638 STB1","Condition":"TM1638_ENABLEDMODULES > 0"}
#define TM1638_SINGLECOLOR1 1   //{"Name":"TM1638_SINGLECOLOR1","Title":"1st module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 0"}

#define TM1638_STB2 0          //{"Name":"TM1638_STB2","Title":"2nd module strobe (STB1) digital pin number","DefaultValue":"10","Type":"pin;TM1638 STB2","Condition":"TM1638_ENABLEDMODULES > 1"}
#define TM1638_SINGLECOLOR2 0  //{"Name":"TM1638_SINGLECOLOR2","Title":"2nd module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 1"}

#define TM1638_STB3 0          //{"Name":"TM1638_STB3","Title":"3rd module strobe (STB2) digital pin number","DefaultValue":"11","Type":"pin;TM1638 STB3","Condition":"TM1638_ENABLEDMODULES > 2"}
#define TM1638_SINGLECOLOR3 0  //{"Name":"TM1638_SINGLECOLOR3","Title":"3rd module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 2"}

#define TM1638_STB4 0          //{"Name":"TM1638_STB4","Title":"4th module strobe (STB3) digital pin number","DefaultValue":"12","Type":"pin;TM1638 STB4","Condition":"TM1638_ENABLEDMODULES > 3"}
#define TM1638_SINGLECOLOR4 0  //{"Name":"TM1638_SINGLECOLOR4","Title":"4th module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 3"}

#define TM1638_STB5 0           //{"Name":"TM1638_STB5","Title":"5th module strobe (STB4) digital pin number","DefaultValue":"0","Type":"pin;TM1638 STB5","Condition":"TM1638_ENABLEDMODULES > 4"}
#define TM1638_SINGLECOLOR5 0   //{"Name":"TM1638_SINGLECOLOR5","Title":"5th module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 4"}

#define TM1638_STB6 0           //{"Name":"TM1638_STB6","Title":"6th module strobe (STB5) digital pin number","DefaultValue":"0","Type":"pin;TM1638 STB6","Condition":"TM1638_ENABLEDMODULES > 5"}
#define TM1638_SINGLECOLOR6 0   //{"Name":"TM1638_SINGLECOLOR6","Title":"6th module is single color","DefaultValue":"0","Type":"bool","Condition":"TM1638_ENABLEDMODULES > 5"}

#include "SHTM1638.h"
#endif

// -------------------------------------------------------------------------------------------------------
// TM1637 Modules ----------------------------------------------------------------------------------------
// http://www.dx.com/p/0-36-led-4-digit-display-module-for-arduino-black-blue-works-with-official-arduino-boards-254978
// -------------------------------------------------------------------------------------------------------
// Number of Connected TM1637 modules
// 0 disabled, > 0 enabled
#define TM1637_ENABLEDMODULES 0 //{"Group":"TM1637 7 Segment Modules","Name":"TM1637_ENABLEDMODULES","Title":"TM1637 modules connected","DefaultValue":"0","Type":"int","Max":8}
#ifdef INCLUDE_TM1637

#define TM1637_DIO1 4           //{"Name":"TM1637_DIO1","Title":"1st TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #1 DIO","Condition":"TM1637_ENABLEDMODULES >=1"}
#define TM1637_CLK1 3           //{"Name":"TM1637_CLK1","Title":"1st TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #1 CLK","Condition":"TM1637_ENABLEDMODULES >=1"}
#define TM1637_DIO2 4           //{"Name":"TM1637_DIO2","Title":"2nd TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #2 DIO","Condition":"TM1637_ENABLEDMODULES >=2"}
#define TM1637_CLK2 3           //{"Name":"TM1637_CLK2","Title":"2nd TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #2 CLK","Condition":"TM1637_ENABLEDMODULES >=2"}
#define TM1637_DIO3 4           //{"Name":"TM1637_DIO3","Title":"3rd TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #3 DIO","Condition":"TM1637_ENABLEDMODULES >=3"}
#define TM1637_CLK3 3           //{"Name":"TM1637_CLK3","Title":"3rd TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #3 CLK","Condition":"TM1637_ENABLEDMODULES >=3"}
#define TM1637_DIO4 4           //{"Name":"TM1637_DIO4","Title":"4th TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #4 DIO","Condition":"TM1637_ENABLEDMODULES >=4"}
#define TM1637_CLK4 3           //{"Name":"TM1637_CLK4","Title":"4th TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #4 CLK","Condition":"TM1637_ENABLEDMODULES >=4"}
#define TM1637_DIO5 4           //{"Name":"TM1637_DIO5","Title":"5th TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #5 DIO","Condition":"TM1637_ENABLEDMODULES >=5"}
#define TM1637_CLK5 3           //{"Name":"TM1637_CLK5","Title":"5th TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #5 CLK","Condition":"TM1637_ENABLEDMODULES >=5"}
#define TM1637_DIO6 4           //{"Name":"TM1637_DIO6","Title":"6th TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #6 DIO","Condition":"TM1637_ENABLEDMODULES >=6"}
#define TM1637_CLK6 3           //{"Name":"TM1637_CLK6","Title":"6th TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #6 CLK","Condition":"TM1637_ENABLEDMODULES >=6"}
#define TM1637_DIO7 4           //{"Name":"TM1637_DIO7","Title":"7th TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #7 DIO","Condition":"TM1637_ENABLEDMODULES >=7"}
#define TM1637_CLK7 3           //{"Name":"TM1637_CLK7","Title":"7th TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #7 CLK","Condition":"TM1637_ENABLEDMODULES >=7"}
#define TM1637_DIO8 4           //{"Name":"TM1637_DIO8","Title":"8th TM1637 DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 #8 DIO","Condition":"TM1637_ENABLEDMODULES >=8"}
#define TM1637_CLK8 3           //{"Name":"TM1637_CLK8","Title":"8th TM1637 CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 #8 CLK","Condition":"TM1637_ENABLEDMODULES >=8"}

#include "SHTM1637.h"

#endif

// -------------------------------------------------------------------------------------------------------
// TM1637 6 DIGITS Modules ----------------------------------------------------------------------------------------
// http://www.dx.com/p/0-36-led-4-digit-display-module-for-arduino-black-blue-works-with-official-arduino-boards-254978
// -------------------------------------------------------------------------------------------------------
// Number of Connected TM1637 modules
// 0 disabled, > 0 enabled
#define TM1637_6D_ENABLEDMODULES 0 //{"Group":"TM1637 6Digits 7 Segment Modules","Name":"TM1637_6D_ENABLEDMODULES","Title":"TM1637 modules connected","DefaultValue":"0","Type":"int","Max":8}
#ifdef INCLUDE_TM1637_6D

#define TM1637_6D_DIO1 4           //{"Name":"TM1637_6D_DIO1","Title":"1st TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #1 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=1"}
#define TM1637_6D_CLK1 3           //{"Name":"TM1637_6D_CLK1","Title":"1st TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #1 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=1"}
#define TM1637_6D_DIO2 4           //{"Name":"TM1637_6D_DIO2","Title":"2nd TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #2 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=2"}
#define TM1637_6D_CLK2 3           //{"Name":"TM1637_6D_CLK2","Title":"2nd TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #2 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=2"}
#define TM1637_6D_DIO3 4           //{"Name":"TM1637_6D_DIO3","Title":"3rd TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #3 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=3"}
#define TM1637_6D_CLK3 3           //{"Name":"TM1637_6D_CLK3","Title":"3rd TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #3 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=3"}
#define TM1637_6D_DIO4 4           //{"Name":"TM1637_6D_DIO4","Title":"4th TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #4 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=4"}
#define TM1637_6D_CLK4 3           //{"Name":"TM1637_6D_CLK4","Title":"4th TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #4 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=4"}
#define TM1637_6D_DIO5 4           //{"Name":"TM1637_6D_DIO5","Title":"5th TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #5 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=5"}
#define TM1637_6D_CLK5 3           //{"Name":"TM1637_6D_CLK5","Title":"5th TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #5 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=5"}
#define TM1637_6D_DIO6 4           //{"Name":"TM1637_6D_DIO6","Title":"6th TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #6 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=6"}
#define TM1637_6D_CLK6 3           //{"Name":"TM1637_6D_CLK6","Title":"6th TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #6 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=6"}
#define TM1637_6D_DIO7 4           //{"Name":"TM1637_6D_DIO7","Title":"7th TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #7 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=7"}
#define TM1637_6D_CLK7 3           //{"Name":"TM1637_6D_CLK7","Title":"7th TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #7 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=7"}
#define TM1637_6D_DIO8 4           //{"Name":"TM1637_6D_DIO8","Title":"8th TM1637 6D DIO digital pin number","DefaultValue":"4","Type":"pin;TM1637 6D #8 DIO","Condition":"TM1637_6D_ENABLEDMODULES >=8"}
#define TM1637_6D_CLK8 3           //{"Name":"TM1637_6D_CLK8","Title":"8th TM1637 6D CLK digital pin number","DefaultValue":"3","Type":"pin;TM1637 6D #8 CLK","Condition":"TM1637_6D_ENABLEDMODULES >=8"}

#include "SHTM1637_6D.h"

#endif

// -------------------------------------------------------------------------------------------------------
// MAX7219 / MAX7221 7 Segment modules -----------------------------------------------------------------------------
// http://www.dx.com/p/max7219-led-dot-matrix-digital-led-display-tube-module-cascade-391256
// -------------------------------------------------------------------------------------------------------
// 0 disabled, > 0 enabled
#define MAX7221_ENABLEDMODULES 0 //{"Group":"MAX7221 7 Segments Modules","Name":"MAX7221_ENABLEDMODULES","Title":"MAX7219 / MAX7221 7 Segment modules connected \r\nMultiple modules can be cascaded connected module output to next module input","DefaultValue":"0","Type":"int","Max":8}
#ifdef INCLUDE_MAX7221_MODULES
#include "SHMAX72217Segment.h"
// USING HARDWARE SPI BY DEFAULT (MOSI, SCK)
#define MAX7221_DATA MOSI        //{"Name":"MAX7221_DATA","Title":"DATA (DIN) digital pin number","DefaultValue":"3","Type":"pin;MAX7221 7seg. DATA","Condition":"MAX7221_ENABLEDMODULES > 0"}
#define MAX7221_CLK SCK          //{"Name":"MAX7221_CLK","Title":"CLOCK (CLK) digital pin number","DefaultValue":"5","Type":"pin;MAX7221 7seg. CLK","Condition":"MAX7221_ENABLEDMODULES > 0"}
#define MAX7221_LOAD 13          //{"Name":"MAX7221_LOAD","Title":"LOAD (LD) digital pin number","DefaultValue":"4","Type":"pin;MAX7221 7seg. LOAD/LD","Condition":"MAX7221_ENABLEDMODULES > 0"}
SHMAX72217Segment shMAX72217Segment;
#endif // INCLUDE_MAX7221_MODULES

// -------------------------------------------------------------------------------------------------------
// MAX7219 / MAX7221 Led Matrix module -----------------------------------------------------------------------------
// http://www.dx.com/p/max7219-dot-matrix-module-w-5-dupont-lines-184854
// -------------------------------------------------------------------------------------------------------
#define MAX7221_MATRIX_ENABLED 0 //{"Group":"MAX7221 Led Matrix","Name":"MAX7221_MATRIX_ENABLED","Title":"MAX7221 led matrix enabled ","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_MAX7221MATRIX
#include "SHMatrixMAX7219.h"
// USING HARDWARE SPI BY DEFAULT (MOSI, SCK, SS)
#define MAX7221_MATRIX_DATA MOSI    //{"Name":"MAX7221_MATRIX_DATA","Title":"DATA (DIN) digital pin number","DefaultValue":"3","Type":"pin;MAX7221 Matrix DATA","Condition":"MAX7221_MATRIX_ENABLED>0"}
#define MAX7221_MATRIX_CLK SCK      //{"Name":"MAX7221_MATRIX_CLK","Title":"CLOCK (CLK) digital pin number","DefaultValue":"5","Type":"pin;MAX7221 Matrix CLK","Condition":"MAX7221_MATRIX_ENABLED>0"}
#define MAX7221_MATRIX_LOAD SS      //{"Name":"MAX7221_MATRIX_LOAD","Title":"LOAD (LD/CS) digital pin number","DefaultValue":"4","Type":"pin;MAX7221 Matrix LOAD/LD","Condition":"MAX7221_MATRIX_ENABLED>0"}
SHMatrixMAX7219 shMatrixMAX7219;
#endif

// --------------------------------------------------------------------------------------------------------
// Adafruit 8x8 single color with HT16K33 I2C Backpack
// These displays use I2C to communicate, 2 pins are required to interface.
// There are multiple selectable I2C addresses.
// For backpacks with 2 Address Select addresses : 0x70, 0x71, 0x72 or 0x73.
// For backpacks with 3 Address Select addresses : 0x70 thru 0x77
// --------------------------------------------------------------------------------------------------------
// 0 disabled, 1 enabled
#define ENABLE_ADA_HT16K33_SingleColorMatrix 0     //{"Group":"HT16K33 Single Color Matrix","Name":"ENABLE_ADA_HT16K33_SingleColorMatrix","Title":"Adafruit HT16K33 single color matrix display enabled","DefaultValue":"0","Type":"bool","Pins":"328:18,HT16K33 SC MATRIX DAT;19,HT16K33 SC MATRIX CLK|micro:2,HT16K33 SC MATRIX DAT;3,HT16K33 SC MATRIX CLK|mega:20,HT16K33 SC MATRIX DAT;21,HT16K33 SC MATRIX CLK"}
// I2C Adress
#ifdef INCLUDE_HT16K33_SINGLECOLORMATRIX
#include "SHMatrixHT16H33SingleColor.h"
#define ADA_HT16K33_SINGLECOLORMATRIX_I2CADDRESS 0x70 //{"Name":"ADA_HT16K33_SINGLECOLORMATRIX_I2CADDRESS","Title":"Adafruit HT16K33 single color matrix matrix I2C address","DefaultValue":"0x70","Type":"hex","Condition":"ENABLE_ADA_HT16K33_SingleColorMatrix>0"}
SHMatrixHT16H33SingleColor shMatrixHT16H33SingleColor;
#endif



// -------------------------------------------------------
// NeopixelBus 
// -------------------------------------------------------
#ifdef INCLUDE_RGB_LEDS_NEOPIXELBUS
// Configure it here!
#include <NeoPixelBusLEDs.h>
#endif


// -------------------------------------------------------
// NeopixelBus Matrix
// -------------------------------------------------------
#ifdef INCLUDE_RGB_MATRIX_NEOPIXELBUS

#endif


// -------------------------------------------------------------------------------------------------------
// WS2812b RGBLEDS ---------------------------------------------------------------------------------------
// http://www.dx.com/p/8-bit-ws2812-5050-rgb-led-development-board-w-built-in-full-color-drive-387667
// -------------------------------------------------------------------------------------------------------
// WS2812b chained RGBLEDS count
// 0 disabled, > 0 enabled
#define WS2812B_RGBLEDCOUNT 0        //{"Group":"WS2812B RGB Leds","Name":"WS2812B_RGBLEDCOUNT","Title":"WS2812B RGB leds count","DefaultValue":"0","Type":"int","Max":150}
#ifdef INCLUDE_WS2812B

#define WS2812B_DATAPIN 33  		 //{"Name":"WS2812B_DATAPIN","Title":"Data (DIN) digital pin number","DefaultValue":"6","Type":"pin;WS2812B LEDS DATA","Condition":"WS2812B_RGBLEDCOUNT>0"}
#define WS2812B_RGBENCODING 0        //{"Name":"WS2812B_RGBENCODING","Title":"WS2812B RGB encoding\r\nSet to 0 for GRB, 1 for RGB encoding, 2 for BRG encoding","DefaultValue":"0","Type":"list","Condition":"WS2812B_RGBLEDCOUNT>0","ListValues":"0,GRB encoding;1,RGB encoding;2,BRG encoding"}
#define WS2812B_RIGHTTOLEFT 0        //{"Name":"WS2812B_RIGHTTOLEFT","Title":"Reverse led order ","DefaultValue":"0","Type":"bool","Condition":"WS2812B_RGBLEDCOUNT>0"}
#define WS2812B_TESTMODE 1           //{"Name":"WS2812B_TESTMODE","Title":"TESTING MODE : Light up all configured leds (in red color) at arduino startup\r\nIt will clear after simhub connection","DefaultValue":"0","Type":"bool","Condition":"WS2812B_RGBLEDCOUNT>0"}
#define WS2812B_USEADAFRUITLIBRARY 0 //{"Name":"WS2812B_USEADAFRUITLIBRARY","Title":"ADVANCED : Use legacy adafruit library (only enable if you have sketch size issues)","DefaultValue":"0","Type":"bool","Condition":"WS2812B_RGBLEDCOUNT>0"}

#if(WS2812B_USEADAFRUITLIBRARY == 0)
#include "SHRGBLedsNeoPixelFastLed.h"
SHRGBLedsNeoPixelFastLeds shRGBLedsWS2812B;
#else
#include "SHRGBLedsNeoPixel.h"
SHRGBLedsNeoPixel shRGBLedsWS2812B;
Adafruit_NeoPixel WS2812B_strip = Adafruit_NeoPixel(WS2812B_RGBLEDCOUNT, WS2812B_DATAPIN, (WS2812B_RGBENCODING == 0 ? NEO_GRB : (WS2812B_RGBENCODING == 1 ? NEO_RGB : NEO_BRG)) + NEO_KHZ800);
#endif
#endif

// -------------------------------------------------------------------------------------------------------
// WS2812b MATRIX ---------------------------------------------------------------------------------------
// http://www.dx.com/p/8-bit-ws2812-5050-rgb-led-development-board-w-built-in-full-color-drive-387667
// -------------------------------------------------------------------------------------------------------
// WS2812b chained RGBLEDS count
// 0 disabled, > 0 enabled
#define WS2812B_MATRIX_ENABLED 0                 //{"Group":"WS2812B RGB Matrix","Name":"WS2812B_MATRIX_ENABLED","Title":"Enable WS2812B 8x8 matrix","DefaultValue":"0","Type":"bool"}

#ifdef INCLUDE_WS2812B_MATRIX
#define WS2812B_MATRIX_DATAPIN 6                 //{"Name":"WS2812B_MATRIX_DATAPIN","Title":"Data (DIN) digital pin number","DefaultValue":"6","Type":"pin;WS2812B Matrix data","Condition":"WS2812B_MATRIX_ENABLED>0"}
#define WS2812B_MATRIX_SERPENTINELAYOUT 0        //{"Name":"WS2812B_MATRIX_SERPENTINELAYOUT","Title":"Serpentine layout matrix","DefaultValue":"0","Type":"bool","Condition":"WS2812B_MATRIX_ENABLED>0"}
#define WS2812B_MATRIX_SERPENTINELAYOUTREVERSE 0 //{"Name":"WS2812B_MATRIX_SERPENTINELAYOUTREVERSE","Title":"Reverse serpentine layout start direction","DefaultValue":"0","Type":"bool","Condition":"WS2812B_MATRIX_ENABLED>0 && WS2812B_MATRIX_SERPENTINELAYOUT>0"}

#include "SHRGBMatrixNeoPixelFastLed.h"
SHRGBMatrixNeoPixelFastLed shRGBMatrixWS2812B;
#endif


// -------------------------------------------------------------------------------------------------------
// PL9823 RGBLEDS ---------------------------------------------------------------------------------------
// https://www.instructables.com/id/Multicolor-Knight-Rider-with-RGB-PL9823-LED-Arduin/
// -------------------------------------------------------------------------------------------------------
// PL9823 chained RGBLEDS count
// 0 disabled, > 0 enabled
#define PL9823_RGBLEDCOUNT 0 //{"Group":"PL9823 RGB Leds","Name":"PL9823_RGBLEDCOUNT","Title":"PL9823 RGB leds count","DefaultValue":"0","Type":"int","Max":150}
#ifdef INCLUDE_PL9823
#include <Adafruit_NeoPixel.h>
#include "SHRGBLedsNeoPixel.h"
#define PL9823_DATAPIN 6     //{"Name":"PL9823_DATAPIN","Title":"Data (DIN) digital pin number","DefaultValue":"6","Type":"pin;PL9823 DATA","Condition":"PL9823_RGBLEDCOUNT>0"}
#define PL9823_RIGHTTOLEFT 0 //{"Name":"PL9823_RIGHTTOLEFT","Title":"Reverse led order","DefaultValue":"0","Type":"bool","Condition":"PL9823_RGBLEDCOUNT>0"}
#define PL9823_TESTMODE 0    //{"Name":"PL9823_TESTMODE","Title":"TESTING MODE : Light up all configured leds (in red color) at arduino startup\r\nIt will clear after simhub connection","DefaultValue":"0","Type":"bool","Condition":"PL9823_RGBLEDCOUNT>0"}
Adafruit_NeoPixel PL9823_strip = Adafruit_NeoPixel(PL9823_RGBLEDCOUNT, PL9823_DATAPIN, NEO_RGB + NEO_KHZ400);
SHRGBLedsNeoPixel shRGBLedsPL9823;
#endif

// -------------------------------------------------------------------------------------------------------
// WS2801 RGBLEDS ----------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
// 0 disabled, > 0 enabled
#define WS2801_RGBLEDCOUNT 0 //{"Group":"WS2801 RGB Leds","Name":"WS2801_RGBLEDCOUNT","Title":"WS2801 RGB leds count","DefaultValue":"0","Type":"int","Max":150}
#ifdef INCLUDE_WS2801
#include "SHRGBLedsWS2801.h"
// 0 leds will be used from left to right, 1 leds will be used from right to left
#define WS2801_RIGHTTOLEFT 0 //{"Name":"WS2801_RIGHTTOLEFT","Title":"WS2801 right to left","DefaultValue":"0","Type":"bool","Condition":"WS2801_RGBLEDCOUNT>0"}
#define WS2801_DATAPIN 5     //{"Name":"WS2801_DATAPIN","Title":"Data (DIN) digital pin number","DefaultValue":"5","Type":"pin;WS2801 DATA","Condition":"WS2801_RGBLEDCOUNT>0"}
#define WS2801_CLOCKPIN 6    //{"Name":"WS2801_CLOCKPIN","Title":"Clock (CLK) digital pin number","DefaultValue":"6","Type":"pin;WS2801 CLOCK","Condition":"WS2801_RGBLEDCOUNT>0"}
#define WS2801_TESTMODE 0    //{"Name":"WS2801_TESTMODE","Title":"TESTING MODE : Light up all configured leds (in red color) at arduino startup\r\nIt will clear after simhub connection","DefaultValue":"0","Type":"bool","Condition":"WS2801_RGBLEDCOUNT>0"}
Adafruit_WS2801 WS2801_strip = Adafruit_WS2801(WS2801_RGBLEDCOUNT, WS2801_DATAPIN, WS2801_CLOCKPIN);
SHRGBLedsWS2801 shRGBLedsWS2801;
#endif

// -------------------------------------------------------------------------------------------------------
// DM163 RGB MATRIX ---------------------------------------------------------------------------------------
// https://www.sunfounder.com/full-color-rgb-led-matrix-driver-shield-rgb-matrix-screen.html
// -------------------------------------------------------------------------------------------------------

#define DM163_MATRIX_ENABLED 0 //{"Group":"DM163 RGB Matrix","Name":"DM163_MATRIX_ENABLED","Title":"Enable DM163 8x8 (Sunfounder, colorduino, Iflag ....) matrix","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_DM163_MATRIX
#include "SHRGBLedsDM163.h"
SHRGBLedsDM163 shRGBLedsDM163;
#endif


// -------------------------------------------------------------------------------------------------------
// SUNFOUNDER SH1.0-4P I2C RGB MATRIX ---------------------------------------------------------------------------------------
// https://www.sunfounder.com/products/rgb-matrix-panel-for-arduino
// -------------------------------------------------------------------------------------------------------

#define SUNFOUNDERSH104P_MATRIX_ENABLED 0 //{"Group":"SUNFOUNDER SH1.0-4P I2C RGB matrix","Name":"SUNFOUNDERSH104P_MATRIX_ENABLED","Title":"Enable SUNFOUNDER SH1.0-4P I2C RGB 8x8 matrix","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_SUNFOUNDERSH104P_MATRIX
#include "SHRGBMatrixSunfounderSH104P.h"
SHRGBMatrixSunfounderSH104P shRGBMatrixSunfounderSH104P;
#endif

// -------------------------------------------------------------------------------------------------------
// I2C LiquidCristal
// http://www.dx.com/p/arduino-iic-i2c-serial-3-2-lcd-2004-module-display-138611#.Vb0QtW7tlBd
// Board				I2C / TWI pins
// Uno, Ethernet, Nano	A4(SDA), A5(SCL)
// Mega2560				20 (SDA), 21 (SCL)
// Leonardo				2 (SDA), 3 (SCL)
// Due					20 (SDA), 21 (SCL), SDA1, SCL1
// -------------------------------------------------------------------------------------------------------
#define I2CLCD_enabled 0   //{"Group":"20x4 I2C LCD","Name":"I2CLCD_enabled","Title":"I2C LCD (2004) enabled\r\nUno, Ethernet, Nano : A4(SDA), A5(SCL)\r\nMega2560 : 20 (SDA), 21 (SCL)\r\nLeonardo : 2 (SDA), 3 (SCL)\r\nDue : 20 (SDA), 21 (SCL), SDA1, SCL1","DefaultValue":"0","Type":"bool","Pins":"328:18,I2C LCD SDA;19,I2C LCD SCL|micro:2,I2C LCD SDA;3,I2C LCD SCL|mega:20,I2C LCD SDA;21,I2C LCD SCL"}
#ifdef INCLUDE_I2CLCD
#define I2CLCD_size 0      //{"Name":"I2CLCD_size","Title":"LCD size","DefaultValue":"0","Type":"list","Condition":"I2CLCD_enabled>0","ListValues":"0,20x4;1,16x2"}
#define I2CLCD_ADDRESS 0x3f //{"Name":"I2CLCD_ADDRESS","Title":"I2C address (0x30, 0x27 ... )","DefaultValue":"0x3f","Type":"hex","Condition":"I2CLCD_enabled>0"}
#define I2CLCD_LIBRARY 0   //{"Name":"I2CLCD_LIBRARY","Title":"I2C library","DefaultValue":"0","Type":"list","Condition":"I2CLCD_enabled>0","ListValues":"0,PCF8574AT (DFRobot);1,PCF8574T"}
#define I2CLCD_TEST 0      //{"Name":"I2CLCD_TEST","Title":"TESTING MODE : Show hello world at startup","DefaultValue":"0","Type":"bool","Condition":"I2CLCD_enabled>0"}

#define I2CLCD_20x4        //{"Name":"I2CLCD_20x4","Type":"autodefine","Condition":"[I2CLCD_size]=0"}
//#define I2CLCD_16x2        //{"Name":"I2CLCD_16x2","Type":"autodefine","Condition":"[I2CLCD_size]=1"}

#ifdef  I2CLCD_20x4
#define I2CLCD_WIDTH 20
#define I2CLCD_HEIGHT 4
#else
#define I2CLCD_WIDTH 16
#define I2CLCD_HEIGHT 2
#endif

#define I2CLCD_PCF8574AT   //{"Name":"I2CLCD_PCF8574AT","Type":"autodefine","Condition":"[I2CLCD_LIBRARY]=0"}
//#define I2CLCD_PCF8574T    //{"Name":"I2CLCD_PCF8574T","Type":"autodefine","Condition":"[I2CLCD_LIBRARY]=1"}

// DF ROBOT - PCF8574AT
#ifdef  I2CLCD_PCF8574AT
#include <LiquidCrystal_I2C.h>
#include <SHI2CLcd_PCF8574AT.h>
LiquidCrystal_I2C I2CLCD(I2CLCD_ADDRESS, I2CLCD_WIDTH, I2CLCD_HEIGHT);
#endif

#ifdef  I2CLCD_PCF8574T
#include "SHI2CLcd_PCF8574T.h"
LiquidCrystal_PCF8574 I2CLCD(I2CLCD_ADDRESS);
#endif

SHI2CLcd shI2CLcd;
#endif

// --------------------------------------------------------------------------------------------------------
// Adafruit 4x7segment with HT16K33 I2C Backpack
// These displays use I2C to communicate, 2 pins are required to interface.
// There are multiple selectable I2C addresses.
// For backpacks with 2 Address Select addresses : 0x70, 0x71, 0x72 or 0x73.
// For backpacks with 3 Address Select addresses : 0x70 thru 0x77
// --------------------------------------------------------------------------------------------------------
// 0 disabled, 1 enabled
#define ENABLE_ADA_HT16K33_7SEGMENTS 0 //{"Group":"HT16K33 7 Segments Module","Name":"ENABLE_ADA_HT16K33_7SEGMENTS","Title":"Adafruit HT16K33 7 segments display enabled","DefaultValue":"0","Type":"bool","Pins":"328:18,HT16K33 7SEG DAT;19,HT16K33 7SEG CLK|micro:2,HT16K33 7SEG DAT;3,HT16K33 7SEG CLK|mega:20,HT16K33 7SEG DAT;21,HT16K33 7SEG CLK"}
#ifdef INCLUDE_LEDBACKPACK
#include "Adafruit_LEDBackpack.h"
// I2C Adress
byte ADA_HT16K33_7SEGMENTS_I2CADDRESS = 0x70;
Adafruit_7segment ADA_HT16K33_7SEGMENTS = Adafruit_7segment();

#endif

// --------------------------------------------------------------------------------------------------------
// Adafruit 8x8 bi color with HT16K33 I2C Backpack
// These displays use I2C to communicate, 2 pins are required to interface.
// There are multiple selectable I2C addresses.
// For backpacks with 2 Address Select addresses : 0x70, 0x71, 0x72 or 0x73.
// For backpacks with 3 Address Select addresses : 0x70 thru 0x77
// --------------------------------------------------------------------------------------------------------
// 0 disabled, 1 enabled
#define ENABLE_ADA_HT16K33_BiColorMatrix 0     //{"Group":"HT16K33 Bi Color Matrix","Name":"ENABLE_ADA_HT16K33_BiColorMatrix","Title":"Adafruit HT16K33 Bicolor matrix display enabled","DefaultValue":"0","Type":"bool","Pins":"328:18,HT16K33 SC MATRIX DAT;19,HT16K33 SC MATRIX CLK|micro:2,HT16K33 SC MATRIX DAT;3,HT16K33 SC MATRIX CLK|mega:20,HT16K33 SC MATRIX DAT;21,HT16K33 SC MATRIX CLK"}
// I2C Adress
#ifdef INCLUDE_LEDBACKPACK
#include "Adafruit_LEDBackpack.h"
#define ADA_HT16K33_BICOLORMATRIX_I2CADDRESS 0x70 //{"Name":"ADA_HT16K33_BICOLORMATRIX_I2CADDRESS","Title":"Adafruit HT16K33 Bicolor matrix matrix I2C address","DefaultValue":"0x70","Type":"hex","Condition":"ENABLE_ADA_HT16K33_SingleColorMatrix>0"}
Adafruit_BicolorMatrix ADA_HT16K33_MATRIX = Adafruit_BicolorMatrix();
byte ADA_HT16K33_Matrix_luminosity = 0;
#endif

// ------------------------ TACHOMETER ----------------------------------------------------------------------
// https://github.com/zegreatclan/AssettoCorsaTools/wiki/Arduino-After-Market-Tach-support
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_TACHOMETER
#include "SHTonePin.h"
#define ENABLE_TACHOMETER 0 //{"Group":"Tachometer","Name":"ENABLE_TACHOMETER","Title":"After market tachometer enabled\r\nSee wiki for wiring instructions","DefaultValue":"0","Type":"bool"}
#define TACHOMETER_PIN 9    //{"Name":"TACHOMETER_PIN","Title":"Tachometer signal pin","DefaultValue":"9","Type":"pin;Tachometer signal","Condition":"ENABLE_TACHOMETER > 0 "}
SHTonePin rpmTonePin;
#endif

// ------------------------ SPEEDO GAUGE --------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-After-Market-Speedo-support
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_SPEEDOGAUGE
#include "SHTonePin.h"
#define ENABLE_SPEEDOGAUGE 0 //{"Group":"Speedometer","Name":"ENABLE_SPEEDOGAUGE","Title":"After market Speedo gauge enabled\r\nSee wiki for wiring instructions","DefaultValue":"0","Type":"bool"}
#define SPEEDO_PIN 4         //{"Name":"SPEEDO_PIN","Title":"Speedometer signal pin","DefaultValue":"4","Type":"pin;Speedometer signal","Template":"","Condition":"ENABLE_SPEEDOGAUGE >0"}
SHTonePin speedoTonePin;
#endif

// ------------------------ BOOST GAUGE ---------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-After-Market-Boost-Gauge
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_BOOSTGAUGE
#define ENABLE_BOOSTGAUGE 0 //{"Group":"Boost Gauge","Name":"ENABLE_BOOSTGAUGE","Title":"After market boost gauge enabled\r\nSee wiki for wiring instructions","DefaultValue":"0","Type":"bool"}
#define BOOST_PIN 5         //{"Name":"BOOST_PIN","Title":"Boost pwm pin","DefaultValue":"5","Type":"pin;Boost signal","Condition":"ENABLE_BOOSTGAUGE >0"}
#include "SHPWMPin.h"
SHPWMPin shBOOSTPIN(BOOST_PIN, true);
#endif

// ------------------------ TEMP GAUGE ---------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-After-Market-Boost-Gauge
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_TEMPGAUGE
#define ENABLE_TEMPGAUGE 0 //{"Group":"Water Temperature Gauge","Name":"ENABLE_TEMPGAUGE","Title":"E36 Temperature gauge enabled\r\nDeprated see wiki","DefaultValue":"0","Type":"bool"}
#define TEMP_PIN 5         //{"Name":"TEMP_PIN","Title":"TEMP pwm pin","DefaultValue":"5","Type":"pin;Temperature signal","Condition":"ENABLE_TEMPGAUGE >0"}
#include "SHPWMPin.h"
SHPWMPin shTEMPPIN(TEMP_PIN, (int)40);
#endif

// ------------------------ FUEL GAUGE ---------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-After-Market-Boost-Gauge
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_FUELGAUGE
#define ENABLE_FUELGAUGE 0 //{"Group":"Fuel Gauge","Name":"ENABLE_FUELGAUGE","Title":"E36 Fuel gauge enabled\r\nDeprated see wiki","DefaultValue":"0","Type":"bool"}
#define FUEL_PIN 5         //{"Name":"FUEL_PIN","Title":"FUEL pwm pin","DefaultValue":"5","Type":"pin;Fuel signal","Condition":"ENABLE_FUELGAUGE >0"}
#include "SHPWMPin.h"
SHPWMPin shFUELPIN(FUEL_PIN, (int)40);
#endif

// ------------------------ CONS GAUGE ---------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-After-Market-Boost-Gauge
// ----------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_CONSGAUGE
#define ENABLE_CONSGAUGE 0 //{"Group":"Consumption Gauge","Name":"ENABLE_CONSGAUGE","Title":"After market consommation gauge enabled\r\n(DO NOT USE, NOT WORKING YET)","DefaultValue":"0","Type":"bool"}
#define CONS_PIN 5         //{"Name":"CONS_PIN","Title":"CONS pwm pin","DefaultValue":"5","Type":"pin;Consommation signal","Condition":"ENABLE_CONSGAUGE >0"}
#include "SHPWMPin.h"
SHPWMPin shCONSPIN(CONS_PIN, 40);
#endif

#ifdef INCLUDE_GAMEPAD

#include "SHGamepadAxis.h"

#define GAMEPAD_AXIS_01_ENABLED 0           //{"Group":"Gamepad analog axis","Name":"GAMEPAD_AXIS_01_ENABLED","Title":"Throttle axis enabled","DefaultValue":"0","Type":"bool"}
#define GAMEPAD_AXIS_01_PIN 0               //{"Name":"GAMEPAD_AXIS_01_PIN","Title":"Throttle axis analog pin","DefaultValue":"0","Type":"pin;Throttle input","Condition":"GAMEPAD_AXIS_01_ENABLED>0"}
#define GAMEPAD_AXIS_01_MINVALUE 0          //{"Name":"GAMEPAD_AXIS_01_MINVALUE","Title":"Throttle axis analog minimum value","DefaultValue":"0","Type":"int","Condition":"GAMEPAD_AXIS_01_ENABLED>0"}
#define GAMEPAD_AXIS_01_MAXVALUE 1024       //{"Name":"GAMEPAD_AXIS_01_MAXVALUE","Title":"Throttle axis analog maximum value","DefaultValue":"1024","Type":"int","Condition":"GAMEPAD_AXIS_01_ENABLED>0"}
#define GAMEPAD_AXIS_01_SAMPLING 10         //{"Name":"GAMEPAD_AXIS_01_SAMPLING","Title":"Throttle axis analog sampling precision","DefaultValue":"10","Type":"list","Condition":"GAMEPAD_AXIS_01_ENABLED>0","ListValues":"10,10 bits(default 0->1024);11,11 bits oversampling (slower 0->2048)"}
#define GAMEPAD_AXIS_01_EXPONENTIALFACTOR 1 //{"Name":"GAMEPAD_AXIS_01_EXPONENTIALFACTOR","Title":"Throttle axis exponential correction","DefaultValue":"1","Type":"double","Condition":"GAMEPAD_AXIS_01_ENABLED>0","dMin":0.1,"dMax":1.9}

#define GAMEPAD_AXIS_02_ENABLED 0           //{"Name":"GAMEPAD_AXIS_02_ENABLED","Title":"Accelerator axis enabled","DefaultValue":"0","Type":"bool"}
#define GAMEPAD_AXIS_02_PIN 0               //{"Name":"GAMEPAD_AXIS_02_PIN","Title":"Accelerator axis analog pin","DefaultValue":"0","Type":"pin;Accelerator input","Condition":"GAMEPAD_AXIS_02_ENABLED>0"}
#define GAMEPAD_AXIS_02_MINVALUE 0          //{"Name":"GAMEPAD_AXIS_02_MINVALUE","Title":"Accelerator axis analog minimum value","DefaultValue":"0","Type":"int","Condition":"GAMEPAD_AXIS_02_ENABLED>0"}
#define GAMEPAD_AXIS_02_MAXVALUE 1024       //{"Name":"GAMEPAD_AXIS_02_MAXVALUE","Title":"Accelerator axis analog maximum value","DefaultValue":"1024","Type":"int","Condition":"GAMEPAD_AXIS_02_ENABLED>0"}
#define GAMEPAD_AXIS_02_SAMPLING 10         //{"Name":"GAMEPAD_AXIS_02_SAMPLING","Title":"Accelerator axis analog sampling precision","DefaultValue":"10","Type":"list","Condition":"GAMEPAD_AXIS_02_ENABLED>0","ListValues":"10,10 bits(default 0->1024);11,11 bits oversampling (slower 0->2048)"}
#define GAMEPAD_AXIS_02_EXPONENTIALFACTOR 1 //{"Name":"GAMEPAD_AXIS_02_EXPONENTIALFACTOR","Title":"Accelerator axis exponential correction","DefaultValue":"1","Type":"double","Condition":"GAMEPAD_AXIS_02_ENABLED>0","dMin":0.1,"dMax":1.9}

#define GAMEPAD_AXIS_03_ENABLED 0           //{"Name":"GAMEPAD_AXIS_03_ENABLED","Title":"Brake axis enabled","DefaultValue":"0","Type":"bool"}
#define GAMEPAD_AXIS_03_PIN 0               //{"Name":"GAMEPAD_AXIS_03_PIN","Title":"Brake axis analog pin","DefaultValue":"0","Type":"pin;Brake input","Condition":"GAMEPAD_AXIS_03_ENABLED>0"}
#define GAMEPAD_AXIS_03_MINVALUE 0          //{"Name":"GAMEPAD_AXIS_03_MINVALUE","Title":"Brake axis analog minimum value","DefaultValue":"0","Type":"int","Condition":"GAMEPAD_AXIS_03_ENABLED>0"}
#define GAMEPAD_AXIS_03_MAXVALUE 1024       //{"Name":"GAMEPAD_AXIS_03_MAXVALUE","Title":"Brake axis analog maximum value","DefaultValue":"1024","Type":"int","Condition":"GAMEPAD_AXIS_03_ENABLED>0"}
#define GAMEPAD_AXIS_03_SAMPLING 10         //{"Name":"GAMEPAD_AXIS_03_SAMPLING","Title":"Brake axis analog sampling precision","DefaultValue":"10","Type":"list","Condition":"GAMEPAD_AXIS_03_ENABLED>0","ListValues":"10,10 bits(default 0->1024);11,11 bits oversampling (slower 0->2048)"}
#define GAMEPAD_AXIS_03_EXPONENTIALFACTOR 1 //{"Name":"GAMEPAD_AXIS_03_EXPONENTIALFACTOR","Title":"Brake axis exponential correction","DefaultValue":"1","Type":"double","Condition":"GAMEPAD_AXIS_03_ENABLED>0","dMin":0.1,"dMax":1.9}

#if(GAMEPAD_AXIS_01_ENABLED == 1)
SHGamepadAxis SHGAMEPADAXIS01(GAMEPAD_AXIS_01_PIN, 0, GAMEPAD_AXIS_01_MINVALUE, GAMEPAD_AXIS_01_MAXVALUE, GAMEPAD_AXIS_01_SAMPLING, GAMEPAD_AXIS_01_EXPONENTIALFACTOR);
#endif

#if(GAMEPAD_AXIS_02_ENABLED == 1)
SHGamepadAxis SHGAMEPADAXIS02(GAMEPAD_AXIS_02_PIN, 1, GAMEPAD_AXIS_02_MINVALUE, GAMEPAD_AXIS_02_MAXVALUE, GAMEPAD_AXIS_02_SAMPLING, GAMEPAD_AXIS_02_EXPONENTIALFACTOR);
#endif

#if(GAMEPAD_AXIS_03_ENABLED == 1)
SHGamepadAxis SHGAMEPADAXIS03(GAMEPAD_AXIS_03_PIN, 2, GAMEPAD_AXIS_03_MINVALUE, GAMEPAD_AXIS_03_MAXVALUE, GAMEPAD_AXIS_03_SAMPLING, GAMEPAD_AXIS_03_EXPONENTIALFACTOR);
#endif

#endif // INCLUDE_GAMEPAD



// ----------------------- ADDITIONAL BUTTONS ---------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Press-Buttons
// ----------------------------------------------------------------------------------------------------------
#define ENABLED_BUTTONS_COUNT 0 //{"Group":"Additional Buttons","Name":"ENABLED_BUTTONS_COUNT","Title":"Additional buttons (directly connected to the arduino, 12 max) buttons count","DefaultValue":"0","Type":"int","Max":12}
#ifdef  INCLUDE_BUTTONS

#define BUTTON_PIN_1 3          //{"Name":"BUTTON_PIN_1","Title":"1'st Additional button digital pin","DefaultValue":"3","Type":"pin;Button 1","Condition":"ENABLED_BUTTONS_COUNT>=1"}
#define BUTTON_WIRINGMODE_1 0   //{"Name":"BUTTON_WIRINGMODE_1","Title":"1'st Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=1","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_1 0    //{"Name":"BUTTON_LOGICMODE_1","Title":"1'st Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=1","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_2 3          //{"Name":"BUTTON_PIN_2","Title":"2'nd Additional button digital pin","DefaultValue":"3","Type":"pin;Button 2","Condition":"ENABLED_BUTTONS_COUNT>=2"}
#define BUTTON_WIRINGMODE_2 0   //{"Name":"BUTTON_WIRINGMODE_2","Title":"2'nd Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=2","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_2 0    //{"Name":"BUTTON_LOGICMODE_2","Title":"2'nd Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=2","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_3 3          //{"Name":"BUTTON_PIN_3","Title":"3'rd Additional button digital pin","DefaultValue":"3","Type":"pin;Button 3","Condition":"ENABLED_BUTTONS_COUNT>=3"}
#define BUTTON_WIRINGMODE_3 0   //{"Name":"BUTTON_WIRINGMODE_3","Title":"3'rd Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=3","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_3 0    //{"Name":"BUTTON_LOGICMODE_3","Title":"3'rd Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=3","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_4 3          //{"Name":"BUTTON_PIN_4","Title":"4'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 4","Condition":"ENABLED_BUTTONS_COUNT>=4"}
#define BUTTON_WIRINGMODE_4 0   //{"Name":"BUTTON_WIRINGMODE_4","Title":"4'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=4","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_4 0    //{"Name":"BUTTON_LOGICMODE_4","Title":"4'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=4","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_5 3          //{"Name":"BUTTON_PIN_5","Title":"5'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 5","Condition":"ENABLED_BUTTONS_COUNT>=5"}
#define BUTTON_WIRINGMODE_5 0   //{"Name":"BUTTON_WIRINGMODE_5","Title":"5'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=5","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_5 0    //{"Name":"BUTTON_LOGICMODE_5","Title":"5'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=5","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_6 3          //{"Name":"BUTTON_PIN_6","Title":"6'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 6","Condition":"ENABLED_BUTTONS_COUNT>=6"}
#define BUTTON_WIRINGMODE_6 0   //{"Name":"BUTTON_WIRINGMODE_6","Title":"6'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=6","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_6 0    //{"Name":"BUTTON_LOGICMODE_6","Title":"6'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=6","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_7 3          //{"Name":"BUTTON_PIN_7","Title":"7'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 7","Condition":"ENABLED_BUTTONS_COUNT>=7"}
#define BUTTON_WIRINGMODE_7 0   //{"Name":"BUTTON_WIRINGMODE_7","Title":"7'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=7","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_7 0    //{"Name":"BUTTON_LOGICMODE_7","Title":"7'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=7","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_8 3          //{"Name":"BUTTON_PIN_8","Title":"8'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 8","Condition":"ENABLED_BUTTONS_COUNT>=8"}
#define BUTTON_WIRINGMODE_8 0   //{"Name":"BUTTON_WIRINGMODE_8","Title":"8'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=8","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_8 0    //{"Name":"BUTTON_LOGICMODE_8","Title":"8'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=8","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_9 3          //{"Name":"BUTTON_PIN_9","Title":"9'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 9","Condition":"ENABLED_BUTTONS_COUNT>=9"}
#define BUTTON_WIRINGMODE_9 0   //{"Name":"BUTTON_WIRINGMODE_9","Title":"9'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=9","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_9 0    //{"Name":"BUTTON_LOGICMODE_9","Title":"9'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=9","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_10 3         //{"Name":"BUTTON_PIN_10","Title":"10'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 10","Condition":"ENABLED_BUTTONS_COUNT>=10"}
#define BUTTON_WIRINGMODE_10 0  //{"Name":"BUTTON_WIRINGMODE_10","Title":"10'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=10","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_10 0   //{"Name":"BUTTON_LOGICMODE_10","Title":"10'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=10","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_11 3         //{"Name":"BUTTON_PIN_11","Title":"11'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 11","Condition":"ENABLED_BUTTONS_COUNT>=11"}
#define BUTTON_WIRINGMODE_11 0  //{"Name":"BUTTON_WIRINGMODE_11","Title":"11'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=11","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_11 0   //{"Name":"BUTTON_LOGICMODE_11","Title":"11'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=11","ListValues":"0,Normal;1,Reversed"}

#define BUTTON_PIN_12 3         //{"Name":"BUTTON_PIN_12","Title":"12'th Additional button digital pin","DefaultValue":"3","Type":"pin;Button 12","Condition":"ENABLED_BUTTONS_COUNT>=12"}
#define BUTTON_WIRINGMODE_12 0  //{"Name":"BUTTON_WIRINGMODE_12","Title":"12'th Additional button wiring","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=12","ListValues":"0,Pin to GND;1,VCC to pin"}
#define BUTTON_LOGICMODE_12 0   //{"Name":"BUTTON_LOGICMODE_12","Title":"12'th Additional button logic","DefaultValue":"0","Type":"list","Condition":"ENABLED_BUTTONS_COUNT>=12","ListValues":"0,Normal;1,Reversed"}


int BUTTON_PINS[] = { BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4, BUTTON_PIN_5, BUTTON_PIN_6, BUTTON_PIN_7, BUTTON_PIN_8, BUTTON_PIN_9,BUTTON_PIN_10,BUTTON_PIN_11,BUTTON_PIN_12 };
int BUTTON_WIRING_MODES[] = { BUTTON_WIRINGMODE_1, BUTTON_WIRINGMODE_2, BUTTON_WIRINGMODE_3, BUTTON_WIRINGMODE_4, BUTTON_WIRINGMODE_5, BUTTON_WIRINGMODE_6, BUTTON_WIRINGMODE_7, BUTTON_WIRINGMODE_8, BUTTON_WIRINGMODE_9,BUTTON_WIRINGMODE_10,BUTTON_WIRINGMODE_11,BUTTON_WIRINGMODE_12 };
int BUTTON_LOGIC_MODES[] = { BUTTON_LOGICMODE_1,  BUTTON_LOGICMODE_2,  BUTTON_LOGICMODE_3,  BUTTON_LOGICMODE_4,  BUTTON_LOGICMODE_5,  BUTTON_LOGICMODE_6,  BUTTON_LOGICMODE_7,  BUTTON_LOGICMODE_8,  BUTTON_LOGICMODE_9, BUTTON_LOGICMODE_10, BUTTON_LOGICMODE_11, BUTTON_LOGICMODE_12 };
SHButton button1, button2, button3, button4, button5, button6, button7, button8, button9, button10, button11, button12;
SHButton* BUTTONS[] = { &button1, &button2, &button3, &button4, &button5, &button6, &button7, &button8 , &button9, &button10, &button11, &button12 };

#endif

SHDebouncer ButtonsDebouncer(10);

// ----------------------- ROTARY ENCODERS ------------------------------------------------------------------
// https://www.dx.com/p/ky-040-rotary-encoder-module-brick-sensor-development-for-arduino-avr-pic-420429#.W9BCM0sza0Q
// Rotary encoders with pull-up resistors on the 3 outputs
// ----------------------------------------------------------------------------------------------------------
#define ENABLED_ENCODERS_COUNT 0     //{"Group":"Rotary Encoders","Name":"ENABLED_ENCODERS_COUNT","Title":"Rotary encoders enabled","DefaultValue":"0","Type":"int","Max":8}
#ifdef  INCLUDE_ENCODERS
#include "SHRotaryEncoder.h"

#define ENCODER1_CLK_PIN 7           //{"Name":"ENCODER1_CLK_PIN","Title":"Encoder 1 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 1 CLK","Condition":"ENABLED_ENCODERS_COUNT>0"}
#define ENCODER1_DT_PIN 8            //{"Name":"ENCODER1_DT_PIN","Title":"Encoder 1 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 1 DT","Condition":"ENABLED_ENCODERS_COUNT>0"}
#define ENCODER1_BUTTON_PIN 9        //{"Name":"ENCODER1_BUTTON_PIN","Title":"Encoder 1 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 1 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>0","Min":-1}
#define ENCODER1_ENABLE_PULLUP 0     //{"Name":"ENCODER1_ENABLE_PULLUP","Title":"Encoder 1 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>0"}
#define ENCODER1_REVERSE_DIRECTION 0 //{"Name":"ENCODER1_REVERSE_DIRECTION","Title":"Encoder 1 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>0"}
#define ENCODER1_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER1_ENABLE_HALFSTEPS","Title":"Encoder 1 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=1","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER2_CLK_PIN 11          //{"Name":"ENCODER2_CLK_PIN","Title":"Encoder 2 output A (CLK) pin","DefaultValue":"11","Type":"pin;Encoder 2 CLK","Condition":"ENABLED_ENCODERS_COUNT>1"}
#define ENCODER2_DT_PIN 12           //{"Name":"ENCODER2_DT_PIN","Title":"Encoder 2 output B (DT) pin","DefaultValue":"12","Type":"pin;Encoder 2 DT","Condition":"ENABLED_ENCODERS_COUNT>1"}
#define ENCODER2_BUTTON_PIN 13       //{"Name":"ENCODER2_BUTTON_PIN","Title":"Encoder 2 button (SW) pin","DefaultValue":"13","Type":"pin;Encoder 2 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>1","Min":-1}
#define ENCODER2_ENABLE_PULLUP 0     //{"Name":"ENCODER2_ENABLE_PULLUP","Title":"Encoder 2 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>1"}
#define ENCODER2_REVERSE_DIRECTION 0 //{"Name":"ENCODER2_REVERSE_DIRECTION","Title":"Encoder 2 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>1"}
#define ENCODER2_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER2_ENABLE_HALFSTEPS","Title":"Encoder 2 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=2","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER3_CLK_PIN 7           //{"Name":"ENCODER3_CLK_PIN","Title":"Encoder 3 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 3 CLK","Condition":"ENABLED_ENCODERS_COUNT>2"}
#define ENCODER3_DT_PIN 8            //{"Name":"ENCODER3_DT_PIN","Title":"Encoder 3 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 3 DT","Condition":"ENABLED_ENCODERS_COUNT>2"}
#define ENCODER3_BUTTON_PIN 9        //{"Name":"ENCODER3_BUTTON_PIN","Title":"Encoder 3 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 3 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>2","Min":-1}
#define ENCODER3_ENABLE_PULLUP 0     //{"Name":"ENCODER3_ENABLE_PULLUP","Title":"Encoder 3 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>2"}
#define ENCODER3_REVERSE_DIRECTION 0 //{"Name":"ENCODER3_REVERSE_DIRECTION","Title":"Encoder 3 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>2"}
#define ENCODER3_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER3_ENABLE_HALFSTEPS","Title":"Encoder 3 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=3","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER4_CLK_PIN 7           //{"Name":"ENCODER4_CLK_PIN","Title":"Encoder 4 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 4 CLK","Condition":"ENABLED_ENCODERS_COUNT>3"}
#define ENCODER4_DT_PIN 8            //{"Name":"ENCODER4_DT_PIN","Title":"Encoder 4 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 4 DT","Condition":"ENABLED_ENCODERS_COUNT>3"}
#define ENCODER4_BUTTON_PIN 9        //{"Name":"ENCODER4_BUTTON_PIN","Title":"Encoder 4 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 4 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>3","Min":-1}
#define ENCODER4_ENABLE_PULLUP 0     //{"Name":"ENCODER4_ENABLE_PULLUP","Title":"Encoder 4 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>3"}
#define ENCODER4_REVERSE_DIRECTION 0 //{"Name":"ENCODER4_REVERSE_DIRECTION","Title":"Encoder 4 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>3"}
#define ENCODER4_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER4_ENABLE_HALFSTEPS","Title":"Encoder 4 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=4","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER5_CLK_PIN 7           //{"Name":"ENCODER5_CLK_PIN","Title":"Encoder 5 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 5 CLK","Condition":"ENABLED_ENCODERS_COUNT>4"}
#define ENCODER5_DT_PIN 8            //{"Name":"ENCODER5_DT_PIN","Title":"Encoder 5 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 5 DT","Condition":"ENABLED_ENCODERS_COUNT>4"}
#define ENCODER5_BUTTON_PIN 9        //{"Name":"ENCODER5_BUTTON_PIN","Title":"Encoder 5 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 5 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>4","Min":-1}
#define ENCODER5_ENABLE_PULLUP 0     //{"Name":"ENCODER5_ENABLE_PULLUP","Title":"Encoder 5 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>4"}
#define ENCODER5_REVERSE_DIRECTION 0 //{"Name":"ENCODER5_REVERSE_DIRECTION","Title":"Encoder 5 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>4"}
#define ENCODER5_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER5_ENABLE_HALFSTEPS","Title":"Encoder 5 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=5","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER6_CLK_PIN 7           //{"Name":"ENCODER6_CLK_PIN","Title":"Encoder 6 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 6 CLK","Condition":"ENABLED_ENCODERS_COUNT>5"}
#define ENCODER6_DT_PIN 8            //{"Name":"ENCODER6_DT_PIN","Title":"Encoder 6 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 6 DT","Condition":"ENABLED_ENCODERS_COUNT>5"}
#define ENCODER6_BUTTON_PIN 9        //{"Name":"ENCODER6_BUTTON_PIN","Title":"Encoder 6 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 6 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>5","Min":-1}
#define ENCODER6_ENABLE_PULLUP 0     //{"Name":"ENCODER6_ENABLE_PULLUP","Title":"Encoder 6 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>5"}
#define ENCODER6_REVERSE_DIRECTION 0 //{"Name":"ENCODER6_REVERSE_DIRECTION","Title":"Encoder 6 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>5"}
#define ENCODER6_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER6_ENABLE_HALFSTEPS","Title":"Encoder 6 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=6","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER7_CLK_PIN 7           //{"Name":"ENCODER7_CLK_PIN","Title":"Encoder 7 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 7 CLK","Condition":"ENABLED_ENCODERS_COUNT>6"}
#define ENCODER7_DT_PIN 8            //{"Name":"ENCODER7_DT_PIN","Title":"Encoder 7 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 7 DT","Condition":"ENABLED_ENCODERS_COUNT>6"}
#define ENCODER7_BUTTON_PIN 9        //{"Name":"ENCODER7_BUTTON_PIN","Title":"Encoder 7 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 7 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>6","Min":-1}
#define ENCODER7_ENABLE_PULLUP 0     //{"Name":"ENCODER7_ENABLE_PULLUP","Title":"Encoder 7 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>6"}
#define ENCODER7_REVERSE_DIRECTION 0 //{"Name":"ENCODER7_REVERSE_DIRECTION","Title":"Encoder 7 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>6"}
#define ENCODER7_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER7_ENABLE_HALFSTEPS","Title":"Encoder 7 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=7","ListValues":"0,Full steps;1,Half steps"}

#define ENCODER8_CLK_PIN 7           //{"Name":"ENCODER8_CLK_PIN","Title":"Encoder 8 output A (CLK) pin","DefaultValue":"7","Type":"pin;Encoder 8 CLK","Condition":"ENABLED_ENCODERS_COUNT>7"}
#define ENCODER8_DT_PIN 8            //{"Name":"ENCODER8_DT_PIN","Title":"Encoder 8 output B (DT) pin","DefaultValue":"8","Type":"pin;Encoder 8 DT","Condition":"ENABLED_ENCODERS_COUNT>7"}
#define ENCODER8_BUTTON_PIN 9        //{"Name":"ENCODER8_BUTTON_PIN","Title":"Encoder 8 button (SW) pin","DefaultValue":"9","Type":"pin;Encoder 8 SWITCH","Condition":"ENABLED_ENCODERS_COUNT>7","Min":-1}
#define ENCODER8_ENABLE_PULLUP 0     //{"Name":"ENCODER8_ENABLE_PULLUP","Title":"Encoder 8 enable pullup resistor","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>7"}
#define ENCODER8_REVERSE_DIRECTION 0 //{"Name":"ENCODER8_REVERSE_DIRECTION","Title":"Encoder 8 reverse direction","DefaultValue":"0","Type":"bool","Condition":"ENABLED_ENCODERS_COUNT>7"}
#define ENCODER8_ENABLE_HALFSTEPS 0  //{"Name":"ENCODER8_ENABLE_HALFSTEPS","Title":"Encoder 8 steps mode","DefaultValue":"0","Type":"list","Condition":"ENABLED_ENCODERS_COUNT>=8","ListValues":"0,Full steps;1,Half steps"}

SHRotaryEncoder encoder1, encoder2, encoder3, encoder4, encoder5, encoder6, encoder7, encoder8;
SHRotaryEncoder* SHRotaryEncoders[] = { &encoder1, &encoder2, &encoder3, &encoder4, &encoder5, &encoder6, &encoder7, &encoder8 };
#endif

// ----------------------- ROTARY ENCODERS ------------------------------------------------------------------
// https://www.dx.com/p/ky-040-rotary-encoder-module-brick-sensor-development-for-arduino-avr-pic-420429#.W9BCM0sza0Q
// Rotary encoders with pull-up resistors on the 3 outputs
// ----------------------------------------------------------------------------------------------------------
#define ENABLED_BUTTONMATRIX 0 //{"Group":"Button matrix","Name":"ENABLED_BUTTONMATRIX","Title":"Button matrix enabled","DefaultValue":"0","Type":"bool"}

#define BMATRIX_COLS 3         //{"Name":"BMATRIX_COLS","Title":"Columns","DefaultValue":"3","Type":"int","Condition":"ENABLED_BUTTONMATRIX>0","Min":2,"Max":8}
#define BMATRIX_ROWS 3         //{"Name":"BMATRIX_ROWS","Title":"Rows","DefaultValue":"3","Type":"int","Condition":"ENABLED_BUTTONMATRIX>0","Min":2,"Max":8}

#ifdef  INCLUDE_BUTTONMATRIX
#include "SHButtonMatrix.h"
#define BMATRIX_COL1 17         //{"Name":"BMATRIX_COL1","Title":"Column 1 pin","DefaultValue":"2","Type":"pin;Button M. Col 1","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=1"}
#define BMATRIX_COL2 -1         //{"Name":"BMATRIX_COL2","Title":"Column 2 pin","DefaultValue":"3","Type":"pin;Button M. Col 2","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=2"}
#define BMATRIX_COL3 -1         //{"Name":"BMATRIX_COL3","Title":"Column 3 pin","DefaultValue":"4","Type":"pin;Button M. Col 3","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=3"}
#define BMATRIX_COL4 -1         //{"Name":"BMATRIX_COL4","Title":"Column 4 pin","DefaultValue":"5","Type":"pin;Button M. Col 4","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=4"}
#define BMATRIX_COL5 -1         //{"Name":"BMATRIX_COL5","Title":"Column 5 pin","DefaultValue":"2","Type":"pin;Button M. Col 5","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=5"}
#define BMATRIX_COL6 -1         //{"Name":"BMATRIX_COL6","Title":"Column 6 pin","DefaultValue":"2","Type":"pin;Button M. Col 6","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=6"}
#define BMATRIX_COL7 -1         //{"Name":"BMATRIX_COL7","Title":"Column 7 pin","DefaultValue":"2","Type":"pin;Button M. Col 7","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=7"}
#define BMATRIX_COL8 -1         //{"Name":"BMATRIX_COL8","Title":"Column 8 pin","DefaultValue":"2","Type":"pin;Button M. Col 8","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_COLS>=8"}

#define BMATRIX_ROW1 27         //{"Name":"BMATRIX_ROW1","Title":"Row 1 pin","DefaultValue":"6","Type":"pin;Button M. Row 1","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=1"}
#define BMATRIX_ROW2 -1         //{"Name":"BMATRIX_ROW2","Title":"Row 2 pin","DefaultValue":"7","Type":"pin;Button M. Row 2","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=2"}
#define BMATRIX_ROW3 -1         //{"Name":"BMATRIX_ROW3","Title":"Row 3 pin","DefaultValue":"8","Type":"pin;Button M. Row 3","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=3"}
#define BMATRIX_ROW4 -1         //{"Name":"BMATRIX_ROW4","Title":"Row 4 pin","DefaultValue":"9","Type":"pin;Button M. Row 4","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=4"}
#define BMATRIX_ROW5 -1         //{"Name":"BMATRIX_ROW5","Title":"Row 5 pin","DefaultValue":"2","Type":"pin;Button M. Row 5","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=5"}
#define BMATRIX_ROW6 -1         //{"Name":"BMATRIX_ROW6","Title":"Row 6 pin","DefaultValue":"2","Type":"pin;Button M. Row 6","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=6"}
#define BMATRIX_ROW7 -1         //{"Name":"BMATRIX_ROW7","Title":"Row 7 pin","DefaultValue":"2","Type":"pin;Button M. Row 7","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=7"}
#define BMATRIX_ROW8 -1         //{"Name":"BMATRIX_ROW8","Title":"Row 8 pin","DefaultValue":"2","Type":"pin;Button M. Row 8","Condition":"ENABLED_BUTTONMATRIX>0 && BMATRIX_ROWS>=8"}

byte BMATRIX_COLSDEF[8] = { BMATRIX_COL1, BMATRIX_COL2, BMATRIX_COL3, BMATRIX_COL4, BMATRIX_COL5, BMATRIX_COL6, BMATRIX_COL7, BMATRIX_COL8 };
byte BMATRIX_ROWSDEF[8] = { BMATRIX_ROW1, BMATRIX_ROW2, BMATRIX_ROW3, BMATRIX_ROW4, BMATRIX_ROW5, BMATRIX_ROW6, BMATRIX_ROW7, BMATRIX_ROW8 };

SHButtonMatrix shButtonMatrix;

#endif

// -------------------- SHAKEIT ADA MOTOR SHIELD V2 -------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// --------------------------------------------------------------------------------------------------------
#define ADAMOTORS_SHIELDSCOUNT 0 //{"Group":"SHAKEIT Adafruit Motorshield V2","Name":"ADAMOTORS_SHIELDSCOUNT","Title":"Number of adadruit shields connected (up to 3)","DefaultValue":"0","Type":"int","Max":3}
#ifdef INCLUDE_SHAKEITADASHIELD
#define ADAMOTORS_FREQ 1900      //{"Name":"ADAMOTORS_FREQ","Title":"PWM Frequency of the board (1900hz max)","DefaultValue":"1900","Type":"int","Condition":"ADAMOTORS_SHIELDSCOUNT>0","Min":10,"Max":1900}
#include "SHShakeitAdaMotorShieldV2.h"
SHShakeitAdaMotorShieldV2 shShakeitAdaMotorShieldV2;
#endif

// -------------------- SHAKEIT DUAL VNH5019 Board ------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// ------------------------------------------------------------------------------------------------------
#define DUALVNH5019_ENABLED 0          //{"Group":"SHAKEIT Dual VNH5019","Name":"DUALVNH5019_ENABLED","Title":"Dual VNH5019 board enabled","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_SHAKEIDUALVNH5019
#define DUALVNH5019_REVERSEDIRECTION 0 //{"Name":"DUALVNH5019_REVERSEDIRECTION","Title":"Direction ","DefaultValue":"0","Type":"list","Condition":"DUALVNH5019_ENABLED>0","ListValues":"0,Clockwise;1,Counter clockwise"}
#include "SHShakeitDualVNH5019.h"
SHShakeitDualVNH5019 shShakeitDualVNH5019;
#endif

// -------------------- SHAKEIT MONSTER MOTO BOARD ------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// ------------------------------------------------------------------------------------------------------
#define MOTOMONSTER_ENABLED 0          //{"Group":"SHAKEIT MotoMonster Board","Name":"MOTOMONSTER_ENABLED","Title":"Moto Monster board enabled","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_SHAKEITMOTOMONSTER
#define MOTOMONSTER_REVERSEDIRECTION 0 //{"Name":"MOTOMONSTER_REVERSEDIRECTION","Title":"Direction ","DefaultValue":"0","Type":"list","Condition":"MOTOMONSTER_ENABLED>0","ListValues":"0,Clockwise;1,Counter clockwise"}
#include "SHShakeitMotoMonster.h"
SHShakeitMotoMonster shShakeitMotoMonster;
#endif

// -------------------- SHAKEIT ADA MOTOR SHIELD / DK MOTOR SHIELD ----------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// --------------------------------------------------------------------------------------------------------
#define DKMOTOR_SHIELDSCOUNT 0       //{"Group":"SHAKEIT DK Motor Shield","Name":"DKMOTOR_SHIELDSCOUNT","Title":"DK shield enabled\nDEPRECATED : See wiki","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_SHAKEITDKSHIELD
#define DKMOTOR_USEHUMMINGREDUCING 0 //{"Name":"DKMOTOR_USEHUMMINGREDUCING","Title":"Change the pwm frequency to reduce humming (uno only), may strongly reduce power","DefaultValue":"0","Type":"bool","Condition":"DKMOTOR_SHIELDSCOUNT > 0"}
#include "SHShakeitDKMotorShield.h"
SHShakeitDKMotorShield shShakeitDKMotorShield;
#endif

// -------------------- SHAKEIT L298N BREAKOUT BOARD-- ----------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// --------------------------------------------------------------------------------------------------------
#define L98NMOTORS_ENABLED 0 //{"Group":"SHAKEIT L298N Motors Board","Name":"L98NMOTORS_ENABLED","Title":"L298N motor board enabled","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_SHAKEITL298N
// Motor 1
#define L98N_enA 10          //{"Name":"L98N_enA","Title":"ENA PWM pin","DefaultValue":"10","Type":"pin;L298N ENA","Condition":"L98NMOTORS_ENABLED>=1"}
#define L98N_in1 9           //{"Name":"L98N_in1","Title":"IN1 digital pin","DefaultValue":"9","Type":"pin;L298N IN1","Condition":"L98NMOTORS_ENABLED>=1"}
#define L98N_in2 8           //{"Name":"L98N_in2","Title":"IN2 digital pin","DefaultValue":"8","Type":"pin;L298N IN2","Condition":"L98NMOTORS_ENABLED>=1"}
// motor 2
#define L98N_enB 5           //{"Name":"L98N_enB","Title":"ENB PWM pin","DefaultValue":"5","Type":"pin;L298N ENB","Condition":"L98NMOTORS_ENABLED>=1"}
#define L98N_in3 7           //{"Name":"L98N_in3","Title":"IN3 digital pin","DefaultValue":"7","Type":"pin;L298N IN3","Condition":"L98NMOTORS_ENABLED>=1"}
#define L98N_in4 6           //{"Name":"L98N_in4","Title":"IN4 digital pin","DefaultValue":"6","Type":"pin;L298N IN4","Condition":"L98NMOTORS_ENABLED>=1"}
#include "SHShakeitL298N.h"
SHShakeitL298N shShakeitL298N;
#endif

// -------------------- SHAKEIT PWM OUTPUT ----------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// --------------------------------------------------------------------------------------------------------
#define SHAKEITPWM_ENABLED_MOTORS 0 //{"Group":"SHAKEIT PWM Outputs","Name":"SHAKEITPWM_ENABLED_MOTORS","Title":"ShakeIT direct PWM outputs enabled, for fans, TIP120 or any PWM driven custom boards","DefaultValue":"0","Type":"int","Max":4}
#ifdef INCLUDE_SHAKEITPWM
#define SHAKEITPWM_O1 5             //{"Name":"SHAKEITPWM_O1","Title":"PWM Output 1 pin","DefaultValue":"5","Type":"pin;ShakeIt PWM 1","Condition":"SHAKEITPWM_ENABLED_MOTORS>=1"}
#define SHAKEITPWM_MIN_OUTPUT_O1 0  //{"Name":"SHAKEITPWM_MIN_OUTPUT_O1","Title":"PWM Output 1 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=1","Max":255}
#define SHAKEITPWM_MAX_OUTPUT_O1 255 //{"Name":"SHAKEITPWM_MAX_OUTPUT_O1","Title":"PWM Output 1 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=1","Max":255}
#define SHAKEITPWM_O2 6             //{"Name":"SHAKEITPWM_O2","Title":"PWM Output 2 pin","DefaultValue":"6","Type":"pin;ShakeIt PWM 2","Condition":"SHAKEITPWM_ENABLED_MOTORS>=2"}
#define SHAKEITPWM_MIN_OUTPUT_O2 0  //{"Name":"SHAKEITPWM_MIN_OUTPUT_O2","Title":"PWM Output 2 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=2","Max":255}
#define SHAKEITPWM_MAX_OUTPUT_O2 255 //{"Name":"SHAKEITPWM_MAX_OUTPUT_O2","Title":"PWM Output 2 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=2","Max":255}
#define SHAKEITPWM_O3 9             //{"Name":"SHAKEITPWM_O3","Title":"PWM Output 3 pin","DefaultValue":"9","Type":"pin;ShakeIt PWM 3","Condition":"SHAKEITPWM_ENABLED_MOTORS>=3"}
#define SHAKEITPWM_MIN_OUTPUT_O3 0  //{"Name":"SHAKEITPWM_MIN_OUTPUT_O3","Title":"PWM Output 3 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=3","Max":255}
#define SHAKEITPWM_MAX_OUTPUT_O3 255 //{"Name":"SHAKEITPWM_MAX_OUTPUT_O3","Title":"PWM Output 3 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=3","Max":255}
#define SHAKEITPWM_O4 10            //{"Name":"SHAKEITPWM_O4","Title":"PWM Output 4 pin","DefaultValue":"10","Type":"pin;ShakeIt PWM 4","Condition":"SHAKEITPWM_ENABLED_MOTORS>=4"}
#define SHAKEITPWM_MIN_OUTPUT_O4 0  //{"Name":"SHAKEITPWM_MIN_OUTPUT_O4","Title":"PWM Output 4 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=4","Max":255}
#define SHAKEITPWM_MAX_OUTPUT_O4 255 //{"Name":"SHAKEITPWM_MAX_OUTPUT_O4","Title":"PWM Output 4 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWM_ENABLED_MOTORS>=4","Max":255}

#include "SHShakeitPWM.h"
SHShakeitPWM shShakeitPWM;
#endif

// -------------------- SHAKEIT PWM FANS OUTPUT ----------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-Shake-It
// --------------------------------------------------------------------------------------------------------
#define SHAKEITPWMFANS_ENABLED_MOTORS 3        //{"Group":"SHAKEIT PWM FANS Outputs","Name":"SHAKEITPWMFANS_ENABLED_MOTORS","Title":"ShakeIT direct PWM fans enabled (25khz PWM)\r\nArduino Uno : pins 9 or 10\r\nArduino Leonardo pins : 9, 10 or 11\r\nArduino Mega pins : 11, 12 or 13","DefaultValue":"0","Type":"int","Max":3}

#ifdef INCLUDE_SHAKEITPWMFANS
#define SHAKEITPWMFANS_O1 2                    //{"Name":"SHAKEITPWMFANS_O1","Title":"PWM Output 1 pin","DefaultValue":"9","Type":"pin;ShakeIt PWM Fan 1","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1"}
#define SHAKEITPWMFANS_MIN_OUTPUT_O1 0         //{"Name":"SHAKEITPWMFANS_MIN_OUTPUT_O1","Title":"PWM Output 1 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1","Max":255}
#define SHAKEITPWMFANS_MAX_OUTPUT_O1 255       //{"Name":"SHAKEITPWMFANS_MAX_OUTPUT_O1","Title":"PWM Output 1 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1","Max":255}
#define SHAKEITPWMFANS_RELAY_PIN_01 6          //{"Name":"SHAKEITPWMFANS_RELAY_PIN_01","Title":"PWM Output 1 optional on/off relay pin","DefaultValue":"4","Type":"pin;PWN FAN 1 relay","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1","Min":-1}
#define SHAKEITPWMFANS_RELAY_DELAY_01 2000     //{"Name":"SHAKEITPWMFANS_RELAY_DELAY_01","Title":"PWM Output 1 optional relay off delay (ms)","DefaultValue":"2000","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1 && SHAKEITPWMFANS_RELAY_PIN_01 >0"}
#define SHAKEITPWMFANS_RELAY_REVERSELOGIC_01 0 //{"Name":"SHAKEITPWMFANS_RELAY_REVERSELOGIC_01","Title":"PWM Output 1 optional relay reversed logic\nWhen disabled relay pin will be LOW when the relay must be off,\r\nOtherwise when enabled the pin will be HIGH when the relay is off","DefaultValue":"0","Type":"bool","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=1 && SHAKEITPWMFANS_RELAY_PIN_01 > 0"}

#define SHAKEITPWMFANS_O2 3                    //{"Name":"SHAKEITPWMFANS_O2","Title":"PWM Output 2 pin","DefaultValue":"10","Type":"pin;ShakeIt PWM Fan 2","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2"}
#define SHAKEITPWMFANS_MIN_OUTPUT_O2 0         //{"Name":"SHAKEITPWMFANS_MIN_OUTPUT_O2","Title":"PWM Output 2 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2","Max":255}
#define SHAKEITPWMFANS_MAX_OUTPUT_O2 255       //{"Name":"SHAKEITPWMFANS_MAX_OUTPUT_O2","Title":"PWM Output 2 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2","Max":255}
#define SHAKEITPWMFANS_RELAY_PIN_02 7          //{"Name":"SHAKEITPWMFANS_RELAY_PIN_02","Title":"PWM Output 2 optional on/off relay pin","DefaultValue":"5","Type":"pin;PWN FAN 2 relay","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2","Min":-1}
#define SHAKEITPWMFANS_RELAY_DELAY_02 2000     //{"Name":"SHAKEITPWMFANS_RELAY_DELAY_02","Title":"PWM Output 2 optional relay off delay (ms)","DefaultValue":"2000","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2 && SHAKEITPWMFANS_RELAY_PIN_02 >0"}
#define SHAKEITPWMFANS_RELAY_REVERSELOGIC_02 0 //{"Name":"SHAKEITPWMFANS_RELAY_REVERSELOGIC_02","Title":"PWM Output 2 optional relay reversed logic\nWhen disabled relay pin will be LOW when the relay is off,\r\nOtherwise when enabled the pin will be HIGH when the relay is off","DefaultValue":"0","Type":"bool","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=2 && SHAKEITPWMFANS_RELAY_PIN_02 > 0"}

#define SHAKEITPWMFANS_O3 4                    //{"Name":"SHAKEITPWMFANS_O3","Title":"PWM Output 3 pin","DefaultValue":"11","Type":"pin;ShakeIt PWM Fan 3","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3"}
#define SHAKEITPWMFANS_MIN_OUTPUT_O3 0         //{"Name":"SHAKEITPWMFANS_MIN_OUTPUT_O3","Title":"PWM Output 3 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3","Max":255}
#define SHAKEITPWMFANS_MAX_OUTPUT_O3 255       //{"Name":"SHAKEITPWMFANS_MAX_OUTPUT_O3","Title":"PWM Output 3 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3","Max":255}
#define SHAKEITPWMFANS_RELAY_PIN_03 8          //{"Name":"SHAKEITPWMFANS_RELAY_PIN_03","Title":"PWM Output 3 optional on/off relay pin","DefaultValue":"6","Type":"pin;PWN FAN 3 relay","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3","Min":-1}
#define SHAKEITPWMFANS_RELAY_DELAY_03 2000     //{"Name":"SHAKEITPWMFANS_RELAY_DELAY_03","Title":"PWM Output 3 optional relay off delay (ms)","DefaultValue":"2000","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3 && SHAKEITPWMFANS_RELAY_PIN_03 >0"}
#define SHAKEITPWMFANS_RELAY_REVERSELOGIC_03 0 //{"Name":"SHAKEITPWMFANS_RELAY_REVERSELOGIC_03","Title":"PWM Output 3 optional relay reversed logic\nWhen disabled relay pin will be LOW when the relay is off,\r\nOtherwise when enabled the pin will be HIGH when the relay is off","DefaultValue":"0","Type":"bool","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=3 && SHAKEITPWMFANS_RELAY_PIN_03 > 0"}

#define SHAKEITPWMFANS_O4 5                    //{"Name":"SHAKEITPWMFANS_O4","Title":"PWM Output 4 pin","DefaultValue":"10","Type":"pin;ShakeIt PWM Fan 4","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4"}
#define SHAKEITPWMFANS_MIN_OUTPUT_O4 0         //{"Name":"SHAKEITPWMFANS_MIN_OUTPUT_O4","Title":"PWM Output 4 min (lower values will disable output)","DefaultValue":"0","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4","Max":255}
#define SHAKEITPWMFANS_MAX_OUTPUT_O4 255       //{"Name":"SHAKEITPWMFANS_MAX_OUTPUT_O4","Title":"PWM Output 4 max","DefaultValue":"255","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4","Max":255}
#define SHAKEITPWMFANS_RELAY_PIN_04 9          //{"Name":"SHAKEITPWMFANS_RELAY_PIN_04","Title":"PWM Output 4 optional on/off relay pin","DefaultValue":"7","Type":"pin;PWN FAN 4 relay","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4","Min":-1}
#define SHAKEITPWMFANS_RELAY_DELAY_04 2000     //{"Name":"SHAKEITPWMFANS_RELAY_DELAY_04","Title":"PWM Output 4 optional relay off delay (ms)","DefaultValue":"2000","Type":"int","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4 && SHAKEITPWMFANS_RELAY_PIN_04 >0"}
#define SHAKEITPWMFANS_RELAY_REVERSELOGIC_04 0 //{"Name":"SHAKEITPWMFANS_RELAY_REVERSELOGIC_04","Title":"PWM Output 4 optional relay reversed logic\nWhen disabled relay pin will be LOW when the relay is off,\r\nOtherwise when enabled the pin will be HIGH when the relay is off","DefaultValue":"0","Type":"bool","Condition":"SHAKEITPWMFANS_ENABLED_MOTORS>=4 && SHAKEITPWMFANS_RELAY_PIN_04 > 0"}

#include "SHShakeitPWMFans.h"
SHShakeitPWMFans shShakeitPWMFans;
#endif

// -------------------- OLED GLCD -------------------------------------------------------------------------
// https://github.com/zegreatclan/SimHub/wiki/Arduino-SSD1306-0.96''-Oled-I2C
// --------------------------------------------------------------------------------------------------------

#define ENABLED_OLEDLCD 0 //{"Group":"Oled GLCD","Name":"ENABLED_OLEDLCD","Title":"OLED LCD enabled","DefaultValue":"0","Type":"bool","Pins":"328:18,OLED LCD SDA;19,OLED LCD SCK|mega:20,OLED LCD SDA;21,OLED LCD SCK|micro:2,OLED LCD SDA;3,OLED LCD SCK"}
#ifdef INCLUDE_OLED
#define ENABLED_NOKIALCD 0
#include "SHGLCD_I2COLED.h"
SHGLCD_I2COLED shGLCD;
#endif

// -------------------- NOKIA GLCD -------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------------------------------------
#define ENABLED_NOKIALCD 0 //{"Group":"Nokia GLCD","Name":"ENABLED_NOKIALCD","Title":"Nokia LCD enabled","DefaultValue":"0","Type":"bool","Max":2}

#ifdef INCLUDE_NOKIALCD
#define ENABLED_OLEDLCD 0
#include <Adafruit_PCD8544.h>
#include "SHGLCD_NOKIA.h"
SHGLCD_NOKIA shNOKIA;

#endif

// --------------------------------------------------------------------------------------------------------
// 74HC595 Shift register, one 7seg module for gear indicator ---------------------------------------------
// DIY : http://lucidtronix.com/tutorials/2
// --------------------------------------------------------------------------------------------------------
// 0 disabled, 1 enabled
#define ENABLE_74HC595_GEAR_DISPLAY 0 //{"Group":"74HC595 Gear Display","Name":"ENABLE_74HC595_GEAR_DISPLAY","Title":"74HC595 gear display enabled","DefaultValue":"0","Type":"bool"}
#ifdef INCLUDE_74HC595_GEAR_DISPLAY
// Reverse led state
int RS_74HC595_INVERT = 0;
#define RS_74HC595_DATAPIN 2          //{"Name":"RS_74HC595_DATAPIN","Title":"DATA digital pin number","DefaultValue":"2","Type":"pin","Condition":"ENABLE_74HC595_GEAR_DISPLAY >0"}
#define RS_74HC595_LATCHPIN 3         //{"Name":"RS_74HC595_LATCHPIN","Title":"LATCH digital pin number","DefaultValue":"3","Type":"pin","Condition":"ENABLE_74HC595_GEAR_DISPLAY > 0"}
#define RS_74HC595_CLOCKPIN 4         //{"Name":"RS_74HC595_CLOCKPIN","Title":"CLOCK digital pin number","DefaultValue":"4","Type":"pin","Condition":"ENABLE_74HC595_GEAR_DISPLAY > 0"}
// RS_74HC595 DIGITS
// 0,1,2 ....
byte RS_74HC595_dec_digits[] = { 0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110 };
// Empty
byte RS_74HC595_ClearDigit = 0b00000000;
// R
byte RS_74HC595_ReverseDigit = 0b10001100;
// N
byte RS_74HC595_NeutralDigit = 0b11101100;
#endif // INCLUDE_74HC595_GEAR_DISPLAY

// --------------------------------------------------------------------------------------------------------
// 6c595 Shift register, one 7seg module for gear indicator ---------------------------------------------
// DIY : https://www.xsimulator.net/community/threads/12cm-12v-7segments-gear-indicator-with-gamedash.5747/
// --------------------------------------------------------------------------------------------------------
#ifdef INCLUDE_6c595_GEAR_DISPLAY
#define ENABLE_6C595_GEAR_DISPLAY 0 //{"Group":"6c595 Gear Display","Name":"ENABLE_6C595_GEAR_DISPLAY","Title":"6C595 gear display enabled","DefaultValue":"0","Type":"bool"}
#define RS_6c595_DATAPIN 11         //{"Name":"RS_6c595_DATAPIN","Title":"DATA digital pin number, can't be changed !","DefaultValue":"11","Type":"pin","Condition":"ENABLE_6C595_GEAR_DISPLAY>0"}
#define RS_6c595_LATCHPIN 13        //{"Name":"RS_6c595_LATCHPIN","Title":"LATCH digital pin number, can't be changed !","DefaultValue":"13","Type":"pin","Condition":"ENABLE_6C595_GEAR_DISPLAY>0"}
#define RS_6c595_SLAVEPIN 10        //{"Name":"RS_6c595_SLAVEPIN","Title":"SLAVE digital pin number","DefaultValue":"10","Type":"pin","Condition":"ENABLE_6C595_GEAR_DISPLAY>0"}
byte g_6c595fontArray[] = {
	// dp-a-b-c-d-e-f-g
	0b10100001, // 0
	0b00101000, // 1
	0b11001101, // 2
	0b01101101, // 3
	0b00101011, // 4
	0b01100111, // 5
	0b11100111, // 6
	0b00101100, // 7
	0b01111111, // 8
	0b01111111, // 9
	0b00000000, // OFF empty
	0b10000001, // REVERSE SPEED
};
byte g_6c595LEDarray; //computed order that will be sent to the LED display
#endif

#include "SHLedsBackpack.h"

#ifdef INCLUDE_74HC595_GEAR_DISPLAY
void RS_74HC595_SendData(byte data) {
	// take the latchPin low so
	// the LEDs don't change while you're sending in bits:
	digitalWrite(RS_74HC595_LATCHPIN, LOW);
	// shift out the bits:
	shiftOut(RS_74HC595_DATAPIN, RS_74HC595_CLOCKPIN, MSBFIRST, data);
	//take the latch pin high so the LEDs will light up:
	digitalWrite(RS_74HC595_LATCHPIN, HIGH);
}

void RS_74HC595_SetChar(char c) {
	byte data = 0;
	if (c == ' ') {
		data = RS_74HC595_ClearDigit;
	}
	else if (c == 'R') {
		data = RS_74HC595_ReverseDigit;
	}
	else if (c == 'N') {
		data = RS_74HC595_NeutralDigit;
	}
	else {
		int value = c - '0';
		if (value >= 0 && value < 10) {
			data = RS_74HC595_dec_digits[value];
		}
	}

	if (RS_74HC595_INVERT == 1) {
		data = data ^ 0xFF;
	}
	RS_74HC595_SendData(data);
}

void RS_74HC595_Init() {
	//set pins to output so you can control the shift register
	pinMode(RS_74HC595_DATAPIN, OUTPUT);
	pinMode(RS_74HC595_LATCHPIN, OUTPUT);
	pinMode(RS_74HC595_CLOCKPIN, OUTPUT);
	RS_74HC595_SetChar(' ');
}
#endif // INCLUDE_74HC595_GEAR_DISPLAY

#ifdef INCLUDE_6c595_GEAR_DISPLAY
void RS_6c595_SetChar(char c) {
	byte data = 0;
	if (c == ' ') {
		data = g_6c595fontArray[10];
	}
	else if (c == 'R') {
		data = g_6c595fontArray[11];
	}
	else if (c == 'N') {
		data = g_6c595fontArray[0];
	}
	else {
		int value = c - '0';
		if (value >= 0 && value < 10) {
			data = g_6c595fontArray[value];
		}
	}
	digitalWrite(RS_6c595_SLAVEPIN, LOW); // << RCLK line goes low
	SPI.transfer(data);  //  << SRCLK goes  high-low 8 times to output 8 bits of data
	digitalWrite(RS_6c595_SLAVEPIN, HIGH); // data outputs change on this rising edge << RCLK line goes high to move data into output register
}
#endif //INCLUDE_6c595_GEAR_DISPLAY

#ifdef INCLUDE_GAMEPAD

//initialize an Joystick with 34 buttons;
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
	JOYSTICK_TYPE_JOYSTICK, 128, 0,
	false, false, false, false, false, false,
	false, GAMEPAD_AXIS_01_ENABLED, GAMEPAD_AXIS_02_ENABLED, GAMEPAD_AXIS_03_ENABLED, false);

#endif

// J revision sketch. Don't modify this
#define VERSION 'j'

#include "SHCustomProtocol.h"
SHCustomProtocol shCustomProtocol;
#include "SHCommands.h"
#include "SHCommandsGlcd.h"
unsigned long lastMatrixRefresh = 0;


void idle(bool critical) {
#if INCLUDE_WIFI
	yield();
	ECrowneWifi::flush();
#endif

#if(GAMEPAD_AXIS_01_ENABLED == 1)
	SHGAMEPADAXIS01.read();
#endif
#if(GAMEPAD_AXIS_02_ENABLED == 1)
	SHGAMEPADAXIS02.read();
#endif
#if(GAMEPAD_AXIS_03_ENABLED == 1)
	SHGAMEPADAXIS03.read();
#endif
#ifdef  INCLUDE_ENCODERS
	for (int i = 0; i < ENABLED_ENCODERS_COUNT; i++) {
		SHRotaryEncoders[i]->read();
	}
#endif

#ifdef  INCLUDE_BUTTONMATRIX
	shButtonMatrix.read();
#endif

	if (ButtonsDebouncer.Debounce()) {
		bool changed = false;
#ifdef INCLUDE_BUTTONS
		for (int btnIdx = 0; btnIdx < ENABLED_BUTTONS_COUNT; btnIdx++) {
			BUTTONS[btnIdx]->read();
		}
#endif
#ifdef INCLUDE_TM1638
		for (int i = 0; i < TM1638_ENABLEDMODULES; i++) {
			uint32_t allButtons = TM1638_screens[i]->Screen->getButtons();
			byte k1 = allButtons;
			byte k2 = allButtons >> 8;
			byte k3 = allButtons >> 16; // LED&KEY variant uses this byte only
			TM1638_screens[i]->Buttons = k3; // if your buttons don't work, try other variables

			if (TM1638_screens[i]->Buttons != TM1638_screens[i]->Oldbuttons) {
				changed = true;
				byte mask = 0;
				for (int b = 0; b < 8; b++) {
					mask = 0b1 << b;
					if ((TM1638_screens[i]->Buttons & mask) != (TM1638_screens[i]->Oldbuttons & mask)) {
#ifdef INCLUDE_GAMEPAD

#else
						arqserial.CustomPacketStart(0x04, 3);
						arqserial.CustomPacketSendByte(i + 1);
						arqserial.CustomPacketSendByte(b + 1);
						arqserial.CustomPacketSendByte((TM1638_screens[i]->Oldbuttons & mask) > 0 ? 0 : 1);
						arqserial.CustomPacketEnd();
#endif

					}
				}
			}
			TM1638_screens[i]->Oldbuttons = TM1638_screens[i]->Buttons;
		}
#endif
#ifdef INCLUDE_GAMEPAD
		if (changed) {
			UpdateGamepadState();
		}
#endif
		shCustomProtocol.idle();
	}
}

#ifdef  INCLUDE_ENCODERS
void EncoderPositionChanged(int encoderId, int position, byte direction) {
#ifdef INCLUDE_GAMEPAD
	UpdateGamepadEncodersState(true);
#else
	if (direction < 2) {
		arqserial.CustomPacketStart(0x01, 3);
		arqserial.CustomPacketSendByte(encoderId);
		arqserial.CustomPacketSendByte(direction);
		arqserial.CustomPacketSendByte(position);
		arqserial.CustomPacketEnd();
	}
	else {
		arqserial.CustomPacketStart(0x02, 2);
		arqserial.CustomPacketSendByte(encoderId);
		arqserial.CustomPacketSendByte(direction - 2);
		arqserial.CustomPacketEnd();
	}
#endif
}
#endif

void buttonStatusChanged(int buttonId, byte Status) {
#ifdef INCLUDE_GAMEPAD
	Joystick.setButton(TM1638_ENABLEDMODULES * 8 + buttonId - 1, Status);
	Joystick.sendState();
#else
	arqserial.CustomPacketStart(0x03, 2);
	arqserial.CustomPacketSendByte(buttonId);
	arqserial.CustomPacketSendByte(Status);
	arqserial.CustomPacketEnd();
#endif
}

#ifdef  INCLUDE_BUTTONMATRIX
void buttonMatrixStatusChanged(int buttonId, byte Status) {
#ifdef INCLUDE_GAMEPAD
	Joystick.setButton(TM1638_ENABLEDMODULES * 8 + ENABLED_BUTTONS_COUNT + buttonId - 1, Status);
	Joystick.sendState();
#else
	arqserial.CustomPacketStart(0x03, 2);
	arqserial.CustomPacketSendByte(ENABLED_BUTTONS_COUNT + buttonId);
	arqserial.CustomPacketSendByte(Status);
	arqserial.CustomPacketEnd();
#endif
}
#endif

void setup()
{
#if INCLUDE_WIFI
#if DEBUG_TCP_BRIDGE
	Serial.begin(115200);
#endif
#endif

#if INCLUDE_WIFI
	ECrowneWifi::setup(&outgoingStream, &incomingStream);
#endif

	//#ifdef INCLUDE_TEMPGAUGE
	//	shTEMPPIN.SetValue((int)80);
	//#endif

#ifdef INCLUDE_FUELGAUGE
	shFUELPIN.SetValue((int)80);
#endif

	FlowSerialBegin(19200);

#ifdef INCLUDE_GAMEPAD
	Joystick.begin(false);
#endif

#ifdef INCLUDE_TM1638
	TM1638_Init();
#endif

#ifdef INCLUDE_TM1637
	TM1637_Init();
#endif

#ifdef INCLUDE_TM1637_6D
	TM1637_6D_Init();
#endif

#ifdef INCLUDE_WS2812B

#if(WS2812B_USEADAFRUITLIBRARY == 0)
#include "SHRGBLedsNeoPixelFastLed.h"
	shRGBLedsWS2812B.begin(WS2812B_RGBLEDCOUNT, WS2812B_RIGHTTOLEFT, WS2812B_TESTMODE);
#else
#include "SHRGBLedsNeoPixel.h"
	shRGBLedsWS2812B.begin(&WS2812B_strip, WS2812B_RGBLEDCOUNT, WS2812B_RIGHTTOLEFT, WS2812B_TESTMODE);
#endif
#endif
#ifdef INCLUDE_PL9823
	shRGBLedsPL9823.begin(&PL9823_strip, PL9823_RGBLEDCOUNT, PL9823_RIGHTTOLEFT, PL9823_TESTMODE);
#endif

#ifdef INCLUDE_WS2812B_MATRIX
	if (WS2812B_MATRIX_ENABLED > 0) {
		shRGBMatrixWS2812B.begin(64, false, WS2812B_MATRIX_SERPENTINELAYOUT, WS2812B_MATRIX_SERPENTINELAYOUTREVERSE, false);
	}
#endif
#ifdef INCLUDE_DM163_MATRIX
	if (DM163_MATRIX_ENABLED > 0) {
		shRGBLedsDM163.begin(64, false, false);
	}
#endif
#ifdef INCLUDE_SUNFOUNDERSH104P_MATRIX
	if (SUNFOUNDERSH104P_MATRIX_ENABLED > 0) {
		shRGBMatrixSunfounderSH104P.begin(false);
	}
#endif
#ifdef INCLUDE_WS2801
	shRGBLedsWS2801.begin(&WS2801_strip, WS2801_RGBLEDCOUNT, WS2801_RIGHTTOLEFT, WS2801_TESTMODE);
#endif

#ifdef INCLUDE_MAX7221_MODULES
	shMAX72217Segment.begin(MAX7221_ENABLEDMODULES, MAX7221_DATA, MAX7221_CLK, MAX7221_LOAD);
#endif

#ifdef INCLUDE_MAX7221MATRIX
	shMatrixMAX7219.begin(MAX7221_MATRIX_DATA, MAX7221_MATRIX_CLK, MAX7221_MATRIX_LOAD);
#endif

	// 74HC595 INIT
#ifdef INCLUDE_74HC595_GEAR_DISPLAY
	if (ENABLE_74HC595_GEAR_DISPLAY == 1)
	{
		RS_74HC595_Init();
	}
#endif

#ifdef INCLUDE_TACHOMETER
	rpmTonePin.begin(TACHOMETER_PIN);
#endif

#ifdef INCLUDE_SPEEDOGAUGE
	speedoTonePin.begin(SPEEDO_PIN);
#endif

#ifdef INCLUDE_6c595_GEAR_DISPLAY
	if (ENABLE_6C595_GEAR_DISPLAY == 1) {
		pinMode(RS_6c595_DATAPIN, OUTPUT);
		pinMode(RS_6c595_LATCHPIN, OUTPUT);
		pinMode(RS_6c595_SLAVEPIN, OUTPUT);
		SPI.begin();
		digitalWrite(RS_6c595_SLAVEPIN, LOW); // << RCLK line goes low
		SPI.transfer(g_6c595fontArray[8]);  //  << SRCLK goes  high-low 8 times to output 8 bits of data
		digitalWrite(RS_6c595_SLAVEPIN, HIGH); // data outputs change on this rising edge << RCLK line goes high to move data into output register
	}
#endif

	// LCD INIT
#ifdef INCLUDE_I2CLCD
	shI2CLcd.begin(&I2CLCD, I2CLCD_WIDTH, I2CLCD_HEIGHT, I2CLCD_TEST);
#endif

#ifdef INCLUDE_LEDBACKPACK
	// ADA_HT16K33_7SEGMENTS INIT
	if (ENABLE_ADA_HT16K33_7SEGMENTS == 1) {
		ADA_HT16K33_7SEGMENTS.begin(ADA_HT16K33_7SEGMENTS_I2CADDRESS);
	}

	// ADA_HT16K33_MATRIX INIT
	if (ENABLE_ADA_HT16K33_BiColorMatrix == 1) {
		ADA_HT16K33_MATRIX.begin(ADA_HT16K33_BICOLORMATRIX_I2CADDRESS);
	}
#endif
#ifdef INCLUDE_HT16K33_SINGLECOLORMATRIX
	shMatrixHT16H33SingleColor.begin(ADA_HT16K33_SINGLECOLORMATRIX_I2CADDRESS);
#endif

#ifdef INCLUDE_OLED
	shGLCD.Init();
#endif

#ifdef INCLUDE_NOKIALCD
	shNOKIA.Init();
#endif

#ifdef INCLUDE_BUTTONS
	// EXTERNAL BUTTONS INIT
	for (int btnIdx = 0; btnIdx < ENABLED_BUTTONS_COUNT; btnIdx++) {
		BUTTONS[btnIdx]->begin(btnIdx + 1, BUTTON_PINS[btnIdx], buttonStatusChanged, BUTTON_WIRING_MODES[btnIdx], BUTTON_LOGIC_MODES[btnIdx]);
	}
#endif

#ifdef  INCLUDE_BUTTONMATRIX
	shButtonMatrix.begin(BMATRIX_COLS, BMATRIX_ROWS, BMATRIX_COLSDEF, BMATRIX_ROWSDEF, buttonMatrixStatusChanged);
#endif

#ifdef INCLUDE_SHAKEITDKSHIELD
	if (DKMOTOR_SHIELDSCOUNT > 0) shShakeitDKMotorShield.begin(DKMOTOR_USEHUMMINGREDUCING);
#endif
#ifdef INCLUDE_SHAKEITADASHIELD
	shShakeitAdaMotorShieldV2.begin(ADAMOTORS_SHIELDSCOUNT, ADAMOTORS_FREQ);
#endif
#ifdef  INCLUDE_SHAKEITL298N
	shShakeitL298N.begin(L98N_enA, L98N_enB, L98N_in1, L98N_in2, L98N_in3, L98N_in4);
#endif
#ifdef INCLUDE_SHAKEITMOTOMONSTER
	shShakeitMotoMonster.begin(MOTOMONSTER_REVERSEDIRECTION);
#endif
#ifdef INCLUDE_SHAKEIDUALVNH5019
	shShakeitDualVNH5019.begin(DUALVNH5019_REVERSEDIRECTION);
#endif

#ifdef INCLUDE_SHAKEITPWM
	shShakeitPWM.begin(SHAKEITPWM_ENABLED_MOTORS, SHAKEITPWM_O1, SHAKEITPWM_O2, SHAKEITPWM_O3, SHAKEITPWM_O4);
	shShakeitPWM.setMin(SHAKEITPWM_MIN_OUTPUT_O1, SHAKEITPWM_MIN_OUTPUT_O2, SHAKEITPWM_MIN_OUTPUT_O3, SHAKEITPWM_MIN_OUTPUT_O4);
	shShakeitPWM.setMax(SHAKEITPWM_MAX_OUTPUT_O1, SHAKEITPWM_MAX_OUTPUT_O2, SHAKEITPWM_MAX_OUTPUT_O3, SHAKEITPWM_MAX_OUTPUT_O4);
#endif

#ifdef INCLUDE_SHAKEITPWMFANS
	shShakeitPWMFans.begin(SHAKEITPWMFANS_ENABLED_MOTORS, SHAKEITPWMFANS_O1, SHAKEITPWMFANS_O2, SHAKEITPWMFANS_O3, SHAKEITPWMFANS_O4);
	shShakeitPWMFans.setMin(SHAKEITPWMFANS_MIN_OUTPUT_O1, SHAKEITPWMFANS_MIN_OUTPUT_O2, SHAKEITPWMFANS_MIN_OUTPUT_O3, SHAKEITPWMFANS_MIN_OUTPUT_O4);
	shShakeitPWMFans.setMax(SHAKEITPWMFANS_MAX_OUTPUT_O1, SHAKEITPWMFANS_MAX_OUTPUT_O2, SHAKEITPWMFANS_MAX_OUTPUT_O3, SHAKEITPWMFANS_MAX_OUTPUT_O4);
	shShakeitPWMFans.setRelays(SHAKEITPWMFANS_RELAY_PIN_01, SHAKEITPWMFANS_RELAY_PIN_02, SHAKEITPWMFANS_RELAY_PIN_03, SHAKEITPWMFANS_RELAY_PIN_04,
		SHAKEITPWMFANS_RELAY_DELAY_01, SHAKEITPWMFANS_RELAY_DELAY_02, SHAKEITPWMFANS_RELAY_DELAY_03, SHAKEITPWMFANS_RELAY_DELAY_04,
		SHAKEITPWMFANS_RELAY_REVERSELOGIC_01, SHAKEITPWMFANS_RELAY_REVERSELOGIC_02, SHAKEITPWMFANS_RELAY_REVERSELOGIC_03, SHAKEITPWMFANS_RELAY_REVERSELOGIC_04
	);
#endif

#ifdef  INCLUDE_ENCODERS
	InitEncoders();
#endif

	shCustomProtocol.setup();
	arqserial.setIdleFunction(idle);

#if(GAMEPAD_AXIS_01_ENABLED == 1)
	SHGAMEPADAXIS01.SetJoystick(&Joystick);
#endif
#if(GAMEPAD_AXIS_02_ENABLED == 1)
	SHGAMEPADAXIS02.SetJoystick(&Joystick);
#endif
#if(GAMEPAD_AXIS_03_ENABLED == 1)
	SHGAMEPADAXIS03.SetJoystick(&Joystick);
#endif
#ifdef INCLUDE_GAMEPAD
	Joystick.sendState();
#endif
#ifdef INCLUDE_RGB_LEDS_NEOPIXELBUS
	neoPixelBusBegin();
#endif
}

#ifdef  INCLUDE_ENCODERS
void InitEncoders() {
	if (ENABLED_ENCODERS_COUNT > 0) encoder1.begin(ENCODER1_CLK_PIN, ENCODER1_DT_PIN, ENCODER1_BUTTON_PIN, ENCODER1_REVERSE_DIRECTION, ENCODER1_ENABLE_PULLUP, 1, ENCODER1_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 1) encoder2.begin(ENCODER2_CLK_PIN, ENCODER2_DT_PIN, ENCODER2_BUTTON_PIN, ENCODER2_REVERSE_DIRECTION, ENCODER2_ENABLE_PULLUP, 2, ENCODER2_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 2) encoder3.begin(ENCODER3_CLK_PIN, ENCODER3_DT_PIN, ENCODER3_BUTTON_PIN, ENCODER3_REVERSE_DIRECTION, ENCODER3_ENABLE_PULLUP, 3, ENCODER3_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 3) encoder4.begin(ENCODER4_CLK_PIN, ENCODER4_DT_PIN, ENCODER4_BUTTON_PIN, ENCODER4_REVERSE_DIRECTION, ENCODER4_ENABLE_PULLUP, 4, ENCODER4_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 4) encoder5.begin(ENCODER5_CLK_PIN, ENCODER5_DT_PIN, ENCODER5_BUTTON_PIN, ENCODER5_REVERSE_DIRECTION, ENCODER5_ENABLE_PULLUP, 5, ENCODER5_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 5) encoder6.begin(ENCODER6_CLK_PIN, ENCODER6_DT_PIN, ENCODER6_BUTTON_PIN, ENCODER6_REVERSE_DIRECTION, ENCODER6_ENABLE_PULLUP, 6, ENCODER6_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 6) encoder7.begin(ENCODER7_CLK_PIN, ENCODER7_DT_PIN, ENCODER7_BUTTON_PIN, ENCODER7_REVERSE_DIRECTION, ENCODER7_ENABLE_PULLUP, 7, ENCODER7_ENABLE_HALFSTEPS, EncoderPositionChanged);
	if (ENABLED_ENCODERS_COUNT > 7) encoder8.begin(ENCODER8_CLK_PIN, ENCODER8_DT_PIN, ENCODER8_BUTTON_PIN, ENCODER8_REVERSE_DIRECTION, ENCODER8_ENABLE_PULLUP, 8, ENCODER8_ENABLE_HALFSTEPS, EncoderPositionChanged);
}
#endif

#ifdef INCLUDE_GAMEPAD
void UpdateGamepadState() {
	int btnidx = 0;

#ifdef INCLUDE_TM1638
	for (int i = 0; i < TM1638_ENABLEDMODULES; i++) {
		byte buttonsState = TM1638_screens[i]->Buttons;

		for (int i = 0; i < 8; i++) {
			Joystick.setButton(btnidx, buttonsState & (1 << i));
			btnidx++;
		}
	}
#endif

#ifdef INCLUDE_ENCODERS
	UpdateGamepadEncodersState(false);
#endif

	Joystick.sendState();
}

#ifdef INCLUDE_ENCODERS
void UpdateGamepadEncodersState(bool sendState) {
	int btnidx = TM1638_ENABLEDMODULES * 8 + ENABLED_BUTTONS_COUNT + ENABLED_BUTTONMATRIX * (BMATRIX_COLS * BMATRIX_ROWS);
	unsigned long refTime = millis();
	for (int i = 0; i < ENABLED_ENCODERS_COUNT; i++) {
		uint8_t dir = SHRotaryEncoders[i]->getDirection(MICRO_GAMEPAD_ENCODERPRESSTIME, refTime);
		Joystick.setButton(btnidx, dir == 0);
		Joystick.setButton(btnidx + 1, dir == 1);
		Joystick.setButton(btnidx + 2, SHRotaryEncoders[i]->getPressed());

		btnidx += 3;
	}

	if (sendState)
		Joystick.sendState();
}
#endif
#endif

char loop_opt;
char xactionc;
unsigned long lastSerialActivity = 0;


void loop() {
#if INCLUDE_WIFI
	ECrowneWifi::loop();
#endif

#ifdef INCLUDE_SHAKEITL298N
	shShakeitL298N.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEITMOTOMONSTER
	shShakeitMotoMonster.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEIDUALVNH5019
	shShakeitDualVNH5019.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEITADASHIELD
	shShakeitAdaMotorShieldV2.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEITDKSHIELD
	shShakeitDKMotorShield.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEITPWM
	shShakeitPWM.safetyCheck();
#endif
#ifdef INCLUDE_SHAKEITPWMFANS
	shShakeitPWMFans.safetyCheck();
#endif
#ifdef INCLUDE_GAMEPAD
	UpdateGamepadState();
#endif
	shCustomProtocol.loop();

	// Wait for data
	if (FlowSerialAvailable() > 0) {
		if (FlowSerialTimedRead() == MESSAGE_HEADER)
		{
			lastSerialActivity = millis();
			// Read command
			loop_opt = FlowSerialTimedRead();

			switch(loop_opt) {
				case '1': Command_Hello(); break;
				case '8': Command_SetBaudrate(); break;
				case 'J': Command_ButtonsCount(); break;
				case '2': Command_TM1638Count(); break;
				case 'B': Command_SimpleModulesCount(); break;
				case 'A': Command_Acq(); break;
				case 'N': Command_DeviceName(); break;
				case 'I': Command_UniqueId(); break;
				case '0': Command_Features(); break;
				case '3': Command_TM1638Data(); break;
				case 'V': Command_Motors(); break;
				case 'S': Command_7SegmentsData(); break;
				case '4': Command_RGBLEDSCount(); break;
				case '6': Command_RGBLEDSData(); break;
				case 'R': Command_RGBMatrixData(); break;
				case 'M': Command_MatrixData(); break;
				case 'G': Command_GearData(); break;
				case 'L': Command_I2CLCDData(); break;
				case 'K': Command_GLCDData();  break; // Nokia | OLEDS
				case 'P': Command_CustomProtocolData(); break;
				case 'X':
					String xaction = FlowSerialReadStringUntil(' ', '\n');
					if (xaction == F("list")) Command_ExpandedCommandsList();
					else if (xaction == F("mcutype")) Command_MCUType();
					else if (xaction == F("tach")) Command_TachData();
					else if (xaction == F("speedo")) Command_SpeedoData();
					else if (xaction == F("boost")) Command_BoostData();
					else if (xaction == F("temp")) Command_TempData();
					else if (xaction == F("fuel")) Command_FuelData();
					else if (xaction == F("cons")) Command_ConsData();
					else if (xaction == F("encoderscount")) Command_EncodersCount();
				break;

			}
		}
	}
	if (millis() - lastSerialActivity > 5000) {
		Command_Shutdown();
	}
}
