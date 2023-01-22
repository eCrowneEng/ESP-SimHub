#ifndef __SHRGBLEDSWS2801_H__
#define __SHRGBLEDSWS2801_H__

#include <Arduino.h>
#include "SHRGBLedsBase.h"
#include <Adafruit_WS2801.h>

class SHRGBLedsWS2801 : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;

protected:
	Adafruit_WS2801 * WS2801_strip;
public:

	void begin(Adafruit_WS2801 * strip,int maxLeds, int righttoleft, bool testMode) {
		SHRGBLedsBase::begin(maxLeds, righttoleft);
		WS2801_strip = strip;
		WS2801_strip->begin();
		WS2801_strip->show();

		if (testMode > 0) {
			for (int i = 0; i < maxLeds; i++) {
				WS2801_strip->setPixelColor(i, 255, 0, 0);
				WS2801_strip->show();
			}
		}
	}

	void show() {
		WS2801_strip->show();
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
		WS2801_strip->setPixelColor(lednumber, r, g, b);
	}
	
};

#endif