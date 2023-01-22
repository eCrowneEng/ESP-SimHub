#ifndef __SHRGBMATRIXNEOPIXELFASTLED_H__
#define __SHRGBMATRIXNEOPIXELFASTLED_H__

#ifndef FASTLED_ALLOW_INTERRUPTS
#define FASTLED_ALLOW_INTERRUPTS 0
#endif

#include <Arduino.h>
#include "SHRGBLedsBase.h"
#include <FastLED.h>

CRGB SHRGBMatrixNeoPixelFastLeds_leds[64];

class SHRGBMatrixNeoPixelFastLed : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;
	bool zigzag;
	bool reverseZigzag;
public:

	void begin(int maxLeds, int righttoleft, bool zigZag, bool reverseZigZag, bool testMode) {
		SHRGBLedsBase::begin(maxLeds, righttoleft);
		FastLED.addLeds<NEOPIXEL, WS2812B_MATRIX_DATAPIN>(SHRGBMatrixNeoPixelFastLeds_leds, maxLeds);

		zigzag = zigZag;
		reverseZigzag = reverseZigZag;
		if (testMode > 0) {
			for (int i = 0; i < maxLeds; i++) {
				setPixelColor(i, 255, 0, 0);
			}
		}
		FastLED.show();
	}

	void show() {
		FastLED.show();
		//delay(10);
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
		if (zigzag) {
			int Y = lednumber / 8;
			int X = lednumber % 8;
			if (!reverseZigzag) {
				if (!(Y % 2)) {
					X = 7 - X;
				}
			}
			else {
				if ((Y % 2)) {
					X = 7 - X;
				}
			}
			lednumber = Y * 8 + X;
		}

		SHRGBMatrixNeoPixelFastLeds_leds[lednumber].setRGB(r, g, b);
	}
};

#endif