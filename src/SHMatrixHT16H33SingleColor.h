#ifndef __SHMatrixHT16H33SingleColor_H__
#define __SHMatrixHT16H33SingleColor_H__
#include <Arduino.h>
#include "Adafruit_LEDBackpack.h"

class SHMatrixHT16H33SingleColor {
private:

	Adafruit_8x8matrix ADA_HT16K33_SINGLECOLOR_MATRIX = Adafruit_8x8matrix();
	byte luminosity = 0;
public:

	void begin(int I2CAddress) {
		ADA_HT16K33_SINGLECOLOR_MATRIX.begin(I2CAddress);
		ADA_HT16K33_SINGLECOLOR_MATRIX.clear();
		ADA_HT16K33_SINGLECOLOR_MATRIX.writeDisplay();
	}

	void read()
	{
		ADA_HT16K33_SINGLECOLOR_MATRIX.setBrightness(FlowSerialTimedRead());

		for (int j = 0; j < 8; j++) {
			byte row = FlowSerialTimedRead();
			for (int c = 0; c < 8; c++) {
				ADA_HT16K33_SINGLECOLOR_MATRIX.drawPixel(j, c, ((row & (1 << (8 - c - 1))) > 0) ? LED_ON : LED_OFF);
			}
		}

		ADA_HT16K33_SINGLECOLOR_MATRIX.writeDisplay();
	}
};
#endif