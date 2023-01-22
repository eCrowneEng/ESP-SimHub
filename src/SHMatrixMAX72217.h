#ifndef __SHMATRIXMAX72217_H__
#define __SHMATRIXMAX72217_H__
#include <Arduino.h>
#include <LEDControl.h>

class SHMatrixMAX72217 {
private:

	LedControl* MAX7221;
	byte MAX7221_MATRIX_LUMINOSITY = 15;
public:

	void begin(int DataPin, int ClkPin, int LoadPin) {
		MAX7221 = &LedControl(DataPin, ClkPin, LoadPin, 1);
		MAX7221->shutdown(0, false);
		MAX7221->setIntensity(0, 15);
		MAX7221->clearDisplay(0);
	}

	void read()
	{
		// Wait for display data
		int newIntensity = FlowSerialTimedRead();
		if (newIntensity != MAX7221_MATRIX_LUMINOSITY) {
			MAX7221->setIntensity(0, newIntensity * 2 + 1);
			MAX7221_MATRIX_LUMINOSITY = newIntensity;
		}

		for (int j = 0; j < 8; j++) {
			MAX7221->setRow(0, 7 - j, FlowSerialTimedRead());
		}
	}
};
#endif