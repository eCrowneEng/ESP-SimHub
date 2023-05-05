#ifndef _SHMATRIXMAX7219_H__
#define _SHMATRIXMAX7219_H__
#include <Arduino.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

class SHMatrixMAX7219 {
private:
	MD_MAX72XX mx;
	byte MAX7221_MATRIX_LUMINOSITY = 0;
public:

	SHMatrixMAX7219(int DataPin, int ClkPin, int CsPin) : mx(HARDWARE_TYPE, CsPin, 1) {}

	void begin() {
		mx.begin();
		mx.clear(0, 1);
	}

	void read()
	{
		// Wait for display data
		int newIntensity = FlowSerialTimedRead();
		if (newIntensity != MAX7221_MATRIX_LUMINOSITY) {
			mx.control(MD_MAX72XX::INTENSITY, min(MAX_INTENSITY, newIntensity));
			MAX7221_MATRIX_LUMINOSITY = newIntensity;
		}

		for (int j = 0; j < 8; j++) {
			mx.setRow(7-j, FlowSerialTimedRead());
		}
	}
};
#endif