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
	bool zigzag = false;
	bool reverseZigzag = false;
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

	// Push a full 8x8 frame, row major (index = row * 8 + col), 64 * 3 bytes.
	// Uses the same mapping as the classic protocol, so both render alike.
	void renderFrame(const uint8_t* rgb64x3) {
		for (int i = 0; i < 64; i++) {
			setPixelColor(i, rgb64x3[i * 3], rgb64x3[i * 3 + 1], rgb64x3[i * 3 + 2]);
		}
		show();
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