#ifndef __I2CLCD_H__
#define __I2CLCD_H__

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define I2CLCD_width 20
#define I2CLCD_height 4

class SHI2CLcd {
private:
	LiquidCrystal_I2C * I2CLCD;

public:
	void begin(LiquidCrystal_I2C * I2CLCDInstance) {
		I2CLCD = I2CLCDInstance;
		I2CLCD->init();
		I2CLCD->backlight();
		I2CLCD->print("Hello world...");
		I2CLCD->clear();
	}

	void read() {
		// Skip one byte
		FlowSerialTimedRead();
		int row = FlowSerialTimedRead();
		I2CLCD->setCursor(0, row);
		I2CLCD->print(FlowSerialReadStringUntil('\n'));
	}
};

#endif