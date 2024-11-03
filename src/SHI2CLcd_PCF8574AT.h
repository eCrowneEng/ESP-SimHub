#ifndef __I2CLCD_H__
#define __I2CLCD_H__

#include <Arduino.h>
#include <SHI2CLcdBase.h>

class SHI2CLcd : public SHI2CLcdBase {
private:
	LCD_I2C * I2CLCD;

public:
	void begin(LCD_I2C * I2CLCDInstance, int width, int height, bool test, TwoWire &wirePort) {
		// is this commented out to force the logic in the "else" branch of the base's clase read method?
		//SHI2CLcdBase::begin(width, height, test);
		I2CLCD = I2CLCDInstance;
		I2CLCD->begin();
		I2CLCD->backlight();
		I2CLCD->print("Hello world...");
		if(!test) {
			I2CLCD->clear();
		}
	}


	 void setCursor(int x, int y){
		 I2CLCD->setCursor(x, y);
	 }

	 void print(String s) {
		 I2CLCD->print(s);
	 }

};

#endif