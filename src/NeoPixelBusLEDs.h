/**
 * Original implementation and testing by moriusz: https://github.com/moriusz
*/

#include <typeinfo>
#include <NeoPixelBusLg.h>
#include <string>

/****************************
 * 
 * Configuration Starts here
 * 
 ************************** */

#define LED_COUNT 24
#define RIGHTTOLEFT 0
#define TEST_MODE 1

// LED BRIGHTNESS NANNY
//  Think about why you want to go higher than this?
//  is your power supply ready? are your eyes ready?, is your heat dissipation ready?
//  https://learn.adafruit.com/sipping-power-with-neopixels/insights
//  remember, if you don't have an external power supply, your board or USB may not be able
//  to provide enough power.
// luminance goes from 0-255, UPDATE AT YOUR OWN RISK
#define LUMINANCE_LIMIT 150


// The color order that your LED strip uses
// https://github.com/Makuna/NeoPixelBus/wiki/Neo-Features
#define colorSpec NeoGrbFeature // A three-element color in the order of Green, Red, and then Blue. This is used for SK6812(grb), WS2811, and WS2812.
//#define colorSpec NeoRgbFeature //A three-element color in the order of Red, Green, and then Blue. Some older pixels used this. 
//#define colorSpec NeoBgrFeature //A three-element color in the order of Blue, Red, and then Green.


// Identify your LED model or protocol
// Ws2812x << default for this library, no changes required; WS2812a, WS2812b, WS2812c, etc The most compatible
// Sk6812
// Apa106
// 400kbps << old slower speed standard that started this all
// .. or any of these but inverted.. example: Ws2812xInverted
//
// Then replace Ws2812x in the methods below with your LED Model/protocol


// We use different methods for each type of board based on available features and their limitations
#ifdef ESP32
//****** ESP32 ******
// There are more variations of the methods available in this file
//  if you find that these don't work for you, feel free to read more about these here, and be aware of
//  board specific limitations
// https://github.com/Makuna/NeoPixelBus/wiki/ESP32-NeoMethods


//******
// RMT
// little CPU Usage and low memory but many interrupts run for it and requires hardware buffer
// Supports all pins below GPIO34
//******
#if ( !CONFIG_IDF_TARGET_ESP32S3 ) // https://github.com/Makuna/NeoPixelBus/issues/815 (temporary)
#define method NeoEsp32Rmt0Ws2812xMethod
#endif

//******
// I2S
// little CPU Usage, more memory; Not available for S3 or C3 boards
// Supports any output pin
//******
#if ( !CONFIG_IDF_TARGET_ESP32S2 && !CONFIG_IDF_TARGET_ESP32C3 && !CONFIG_IDF_TARGET_ESP32S3 ) // not supported by these boards 
//#define method NeoEsp32I2s0X8Ws2812xMethod // Uses the I2S 0 peripheral in 8 channel parallel mode
//#define method NeoEsp32I2s0X16Ws2812xMethod // Uses the I2S 0 peripheral in 16 channel parallel mode
//#define method NeoEsp32I2s0Ws2812xMethod // Uses the I2S 0 peripheral
#endif


//******
// BitBang
// Uses a lot of CPU, and interrupts such as the ones ran for WiFi make it unstable.
// Supports all pins below GPIO32
//******
//#define method NeoEsp32BitBangWs2812xMethod


// Pick your GPIO pin based on the limitations of the selected method above
#define DATA_PIN 8

#else

//****** ESP8266 ******
// There are other methods, but We're picking the most convenient ones here.
//  Feel free to investigate the others, understand their drawbacks and use them if you want
// https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods

// DMA (I2S)
// FASTEST, BUT only over WIFI AND you cannot receive serial data, only send
// Only GPIO3 (usually named as RX, RDX0)
//  this method requires that we initialize serial before the strip
#if CONNECTION_TYPE != SERIAL
// #define method NeoEsp8266DmaWs2812xMethod
#endif


// UART
// FASTER; 
// Only GPIO2 ("D4" in nodemcu, d1Mini and others, but verify)
#define method NeoEsp8266Uart1Ws2812xMethod
// -- There are other UART methods, that may or may not break serial, this is the safest.


// BitBang
// SLOWEST and least stable over WiFi; 
// pins 0-15 (raw gpio number, not the D{1}, D{2} numbers)
//#define method NeoEsp8266BitBangWs2812xMethod


// IF using DMA, this will be ignored and only GPIO3 will be used
// IF using UART, this will be ignored and only GPIO2 will be used
#define DATA_PIN 2
#endif

// Initial color to fill the strip before SimHub connects to the device 
// R, G, B format from 0-255.. 
//  Be aware that (255, 255, 255) may consume a lot of current
//  more than your device can provide, which can damage it. Start with lower numbers 
//  ex: (50, 0, 0) is red and (100, 0, 0) is still red, just brighter
//  See this: https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels#estimating-power-requirements-2894486
//
// note: that this color is not limited by the luminance limit
auto initialColor = RgbColor(120, 0, 0);


/*************************
 * 
 * Configuration ends here
 * 
 ********************** */

// Instantiate an LED Strip
NeoPixelBusLg<colorSpec, method, NeoGammaTableMethod> neoLedStrip(LED_COUNT, DATA_PIN);


/**
 * Initialization function: prepares the strip and other related things
 */
void neoPixelBusBegin()
{
#if ESP8266 && CONNECTION_TYPE != SERIAL
const std::type_info &classType = typeid(method);
const char *className = classType.name();
const char *prefix = "NeoEsp8266Dma";
if (std::string(className).find(prefix) == 0) {
    Serial.begin(115200);
    while (!Serial); // wait for serial attach
    Serial.println("enabling serial due to the neopixelbus method used");
}
#endif
    neoLedStrip.Begin();
    neoLedStrip.Show();

    if (TEST_MODE)
    {
        for (int i = 0; i < LED_COUNT; i++)
        {
            neoLedStrip.SetPixelColor(i, initialColor);
        }
        neoLedStrip.Show();
    }
    neoLedStrip.SetLuminance(LUMINANCE_LIMIT);
}

void neoPixelBusRead()
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t b1;
    uint16_t b2;
    uint8_t j;
    int mode = 1;
    mode = FlowSerialTimedRead();
    while (mode > 0)
    {
        // Read all
        if (mode == 1)
        {
            for (j = 0; j < LED_COUNT; j++)
            {
                r = FlowSerialTimedRead();
                g = FlowSerialTimedRead();
                b = FlowSerialTimedRead();

                if (RIGHTTOLEFT == 1)
                {
                    neoLedStrip.SetPixelColor(LED_COUNT - j - 1, RgbColor(r, g, b));
                }
                else
                {
                    neoLedStrip.SetPixelColor(j, RgbColor(r, g, b));
                }
            }
        }

        // partial led data
        else if (mode == 2)
        {
            int startled = FlowSerialTimedRead();
            int numleds = FlowSerialTimedRead();

            for (j = startled; j < startled + numleds; j++)
            {
                r = FlowSerialTimedRead();
                g = FlowSerialTimedRead();
                b = FlowSerialTimedRead();

                if (RIGHTTOLEFT == 1)
                {
                    neoLedStrip.SetPixelColor(LED_COUNT - j - 1, RgbColor(r, g, b));
                }
                else
                {
                    neoLedStrip.SetPixelColor(j, RgbColor(r, g, b));
                }
            }
        }

        // repeated led data
        else if (mode == 3)
        {
            int startled = FlowSerialTimedRead();
            int numleds = FlowSerialTimedRead();

            r = FlowSerialTimedRead();
            g = FlowSerialTimedRead();
            b = FlowSerialTimedRead();

            for (j = startled; j < startled + numleds; j++)
            {
                if (RIGHTTOLEFT == 1)
                {
                    neoLedStrip.SetPixelColor(LED_COUNT - j - 1, RgbColor(r, g, b));
                }
                else
                {
                    neoLedStrip.SetPixelColor(j, RgbColor(r, g, b));
                }
            }
        }

        mode = FlowSerialTimedRead();
    }
}

void neoPixelBusShow() {
    if (LED_COUNT > 0 && neoLedStrip.IsDirty()) {
        neoLedStrip.Show();
    }
}

int neoPixelBusCount() {
    return LED_COUNT;
}
