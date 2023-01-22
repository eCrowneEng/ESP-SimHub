#ifndef _SHMATRIXMAX7219_H__
#define _SHMATRIXMAX7219_H__
#include <Arduino.h>
#include "SHLedControl.h"

class SHMatrixMAX7219 {
private:

	SHLedControl  MAX7221;
	byte MAX7221_MATRIX_LUMINOSITY = 0;
public:

	void begin(int DataPin, int ClkPin, int LoadPin) {
		MAX7221.begin(DataPin, ClkPin, LoadPin, 1);
		MAX7221.shutdown(0, false);
		MAX7221.setIntensity(0, 0);
		MAX7221.clearDisplay(0);
	}

	void read()
	{
		// Wait for display data
		int newIntensity = FlowSerialTimedRead();
		if (newIntensity != MAX7221_MATRIX_LUMINOSITY) {
			MAX7221.setIntensity(0, min(15,newIntensity));
			MAX7221_MATRIX_LUMINOSITY = newIntensity;
		}

		for (int j = 0; j < 8; j++) {
			MAX7221.setRow(0, 7 - j, FlowSerialTimedRead());
		}
	}
};
#endif