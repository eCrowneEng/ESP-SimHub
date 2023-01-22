#ifndef __SHRGBLEDSDM163_H__
#define __SHRGBLEDSDM163_H__


#include <Arduino.h>
#include "SHRGBLedsBase.h"
#include <Adafruit_NeoPixel.h>
#include <Colorduino.h>

class SHRGBLedsDM163 : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;

protected:
	
public:

	void begin(int maxLeds, int righttoleft, bool testMode) {
		SHRGBLedsBase::begin(maxLeds, righttoleft);

		Colorduino.Init(); // initialize the board
		unsigned char balance[3] = { 36, 45, 63 };
		Colorduino.SetWhiteBal(balance);

		Colorduino.FlipPage(true);

	}

	void show() {
		Colorduino.FlipPage(true);
		//delay(450);
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
		Colorduino.SetPixel(lednumber % 8, (int)lednumber / 8, r, g, b);
	}
	
};

#endif