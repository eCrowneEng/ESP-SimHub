#include <typeinfo>
#include <NeoPixelBusLg.h>
#define LED_COUNT 24
#define RIGHTTOLEFT 0
#define TEST_MODE 1

// There are many color models. The most common ones will use GRB. 
//  If you're using other types feel free to read more here: 
// https://github.com/Makuna/NeoPixelBus/wiki/Neo-Features
#define colorSpec NeoGrbFeature // A three-element color in the order of Green, Red, and then Blue. This is used for SK6812(grb), WS2811, and WS2812.


// We use different methods for each type of board based on available features and their limitations
#if ESP32
//****** ESP32 ******
// There are more variations of the methods available in this file
//  if you find that these don't work for you, feel free to read more about these here, and be aware of
//  board specific limitations
// https://github.com/Makuna/NeoPixelBus/wiki/ESP32-NeoMethods

//******
// I2S
// little CPU Usage, more memory; Not available for S3 or C3 boards
// Supports any output pin
//******
#if (!USING_ESP32_S3 && !USING_ESP32_C3) // not supported by these boards 
#define method NeoEsp32I2s0Ws2812xMethod // Uses the I2S 0 peripheral ESP32S2 only supports one I2S
#endif

//******
// Parallel Channels
// medium CPU usage and memory
// Supports any output pin
//******
// #define method NeoEsp32I2s0X8Ws2812xMethod //Uses the I2S 0 peripheral in 8 channel parallel mode
// #define method NeoEsp32I2s0X16Ws2812xMethod //Uses the I2S 0 peripheral in 16 channel parallel mode

//******
// RMT
// little CPU Usage and low memory but many interrupts run for it and requires hardware buffer
// Supports all pins below GPIO34
//******
// #define method NeoEsp32Rmt0Ws2812xMethod

//******
// BitBang
// Uses a lot of CPU and interrupts such as the ones ran for WiFi make it unstable.
// Supports all pins below GPIO32
//******
// #define method NeoEsp32BitBangWs2812xMethod

// Pick your GPIO pin based on the limitations of the selected method above
#define DATA_PIN 2

#else

//****** ESP8266 ******
// There are many methods, but the most convenient are UART1 or bitbang
//  Feel free to investigate the others, understand their drawbacks and implement them if you want
// https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods

// DMA (I2S)
// FASTEST, BUT only over WIFI AND you cannot receive serial data, only send
// Only GPIO3 (usually named as RX, RDX0)
#if INCLUDE_WIFI
// #define method NeoEsp8266DmaWs2812xMethod
#endif

// UART
// FASTER; Only GPIO2 ("D4" in nodemcu, d1Mini and others, but verify)
#define method NeoEsp8266Uart1Ws2812xMethod

// BitBang
// SLOWEST and least stable over WiFi; pins 0-15 (raw gpio number, not the D{1}, D{2} numbers)
//#define method NeoEsp8266BitBangWs2812xMethod

// IF using DMA, this will be ignored and only GPIO3 will be used
// IF using UART, this will be ignored and only GPIO2 will be used
#define DATA_PIN 2
#endif

NeoPixelBusLg<colorSpec, method, NeoGammaTableMethod> neoLedStrip(LED_COUNT, DATA_PIN);

void neoPixelBusBegin()
{
#if ESP8266 && INCLUDE_WIFI
if (typeid(method).name() == typeid(NeoEsp8266DmaWs2812xMethod).name()) {
    Serial.begin(115200);
    while (!Serial); // wait for serial attach
    Serial.println("enabling serial due to the neopixelbus method used");
}
#endif
    neoLedStrip.Begin();
    neoLedStrip.Show();

    if (TEST_MODE)
    {
        neoLedStrip.SetLuminance(155);
        for (int i = 0; i < LED_COUNT; i++)
        {
            neoLedStrip.SetPixelColor(i, RgbColor(120, 0, 0));
        }
        neoLedStrip.Show();
    }
    neoLedStrip.SetLuminance(255);
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
                /*	if (ENABLE_BLUETOOTH == 0) {*/
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