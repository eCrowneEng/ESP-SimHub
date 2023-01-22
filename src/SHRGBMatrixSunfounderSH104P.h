#ifndef __SHRGBMATRIXSUNFOUNDERSH104P_H__
#define __SHRGBMATRIXSUNFOUNDERSH104P_H__

#include <Arduino.h>
#include "SHRGBLedsBase.h"
#include "sunfounder_rgbMatrix.h"



class SHRGBMatrixSunfounderSH104P : public SHRGBLedsBase {
private:
	unsigned long lastRead = 0;

public:

	void begin(bool testMode) {
		SHRGBLedsBase::begin(64,false);
		RGBMatrixInit();
		if (testMode > 0) {
			for (int i = 0; i < 64; i++) {
				setPixelColor(i, 255, 0, 0);
			}
		}
		
	}

	void show() {
		image();
	}

protected:
	void setPixelColor(uint8_t lednumber, uint8_t r, uint8_t g, uint8_t b) {
	
		draw_point(lednumber, r, g, b);
		
	}
};

#endif