#ifndef _SHMATRIXMAX7219_H__
#define _SHMATRIXMAX7219_H__
#include <Arduino.h>
#include <LedController.hpp>

class SHMatrixMAX7219 {
private:
	byte MAX7221_MATRIX_LUMINOSITY = 0;
	LedController MAX7221;
public:
	void begin(int dataPin, int clkPin, int csPin) {
		MAX7221.init(dataPin, clkPin, csPin, 1);
		for(unsigned int i = 0; i < MAX7221.getSegmentCount(); i++) {
			for(unsigned int j = 0; j < 8; j++) {
				MAX7221.setRow(i, j, 0x00);
			}
		}
		MAX7221.setIntensity(15);
	}

	void read()
	{
		int newIntensity = FlowSerialTimedRead();
		if (newIntensity != MAX7221_MATRIX_LUMINOSITY) {
			MAX7221.setIntensity(newIntensity);
			MAX7221_MATRIX_LUMINOSITY = newIntensity;
		}

		for (int j = 0; j < 8; j++) {
			MAX7221.setRow(0, 7 - j, FlowSerialTimedRead());
		}
	}
};
#endif