#ifndef __SHRGBLEDSNEOPIXEL_H__
#define __SHRGBLEDSNEOPIXEL_H__

#include <Arduino.h>
#include "SHRGBLedsBase.h"
#include <Adafruit_NeoPixel.h>

class SHRGBLedsNeoPixel : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;

protected:
	Adafruit_NeoPixel * NeoPixel_strip;
public:

	void begin(Adafruit_NeoPixel * strip,int maxLeds, int righttoleft, bool testMode) {
		SHRGBLedsBase::begin(maxLeds, righttoleft);
		NeoPixel_strip = strip;
		NeoPixel_strip->begin();
		NeoPixel_strip->show();

		if (testMode > 0) {
			for (int i = 0; i < maxLeds; i++) {
				NeoPixel_strip->setPixelColor(i, 255, 0, 0);
				NeoPixel_strip->show();
			}
		}
	}

	void show() {
		NeoPixel_strip->show();
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
		NeoPixel_strip->setPixelColor(lednumber, r, g, b);
	}
	
};

#endif