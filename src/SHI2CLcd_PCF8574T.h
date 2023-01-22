#ifndef __I2CLCD_H__
#define __I2CLCD_H__

#include <Arduino.h>
#include "SHI2CLcdBase.h"

#include <Wire.h>
#include "LiquidCrystal_PCF8574.h"

class SHI2CLcd : public SHI2CLcdBase {
private:
	LiquidCrystal_PCF8574 * I2CLCD;

public:
	void begin(LiquidCrystal_PCF8574 * I2CLCDInstance, int width, int height, bool test) {
		SHI2CLcdBase::begin(width, height, test);
		I2CLCD = I2CLCDInstance;
		I2CLCD->begin(width, height); // initialize the lcd
		I2CLCD->setBacklight(255);
		
		I2CLCD->print("Hello world...");

		if (!test)
			I2CLCD->clear();
	}

	void setCursor(int x, int y) {
		I2CLCD->setCursor(x, y);
	}

	void print(String s) {
		I2CLCD->print(s);
	}
};

#endif