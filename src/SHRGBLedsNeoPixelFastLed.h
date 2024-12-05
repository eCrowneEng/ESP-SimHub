#ifndef __SHRGBLEDSNEOPIXELFASTLED_H__
#define __SHRGBLEDSNEOPIXELFASTLED_H__
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <Arduino.h>
#include <SHRGBLedsBase.h>
#include <FastLED.h>

CRGB SHRGBLedsNeoPixelFastLeds_leds[WS2812B_RGBLEDCOUNT];

class SHRGBLedsNeoPixelFastLeds : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;

public:

	void begin(int maxLeds, int righttoleft, bool testMode) {
		SHRGBLedsBase::begin(maxLeds, righttoleft);
		FastLED.setCorrection(TypicalLEDStrip);
		FastLED.addLeds<NEOPIXEL, WS2812B_DATAPIN>(SHRGBLedsNeoPixelFastLeds_leds, maxLeds);
		if (testMode > 0) {
			for (int i = 0; i < maxLeds; i++) {
				setPixelColor(i, 255, 0, 0);
			}
		}
		show();
	}

	void show() {
		yield();
		FastLED.show();
		yield();
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
		// 0 for GRB, 
		// 1 for RGB encoding
		// 2 for BRG encoding
		if (WS2812B_RGBENCODING == 0) {
			SHRGBLedsNeoPixelFastLeds_leds[lednumber].setRGB(r, g, b);
		}
		else if (WS2812B_RGBENCODING == 1) {
			SHRGBLedsNeoPixelFastLeds_leds[lednumber].setRGB(g, r, b);
		}
		else if (WS2812B_RGBENCODING == 2) {
			SHRGBLedsNeoPixelFastLeds_leds[lednumber].setRGB(b, g, r);
		}
	}
};

#endif