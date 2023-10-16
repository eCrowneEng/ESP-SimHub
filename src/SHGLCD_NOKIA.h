#ifndef __SHGLCD_NOKIA_H__
#define __SHGLCD_NOKIA_H__

#include <Arduino.h>
#include "Adafruit_GFX.h"
#include <Adafruit_PCD8544.h>
#include "SHGLCD_base.h"


// Adafruit_PCD8544 display = Adafruit_PCD8544(SCLK, DIN, DC, CS or SCE, RST);
#pragma message("Make sure these pins are configured properly");
Adafruit_PCD8544 nokia1 = Adafruit_PCD8544(0, 1, 2, 4, 3);
Adafruit_PCD8544 * nokia[] = { &nokia1 };

class SHGLCD_NOKIA : public SHGLCD_Base
{
public:

	void Init() {
		nokia1.begin();
		nokia1.clearDisplay();
		nokia1.setRotation(2);
		//nokiaLCD.setFont(&DJB_Get_Digital15pt7b);
		nokia1.setContrast(50);
		nokia1.setCursor(0, 20);
		nokia1.setFont();
		nokia1.setTextSize(1);

		nokia1.print("Hello");

		nokia1.display();
		nokia1.setCursor(0, 20);
	}

	void Display(int idx) {
		nokia[idx]->display();
	}

	void ClearDisplay(int idx) {
		nokia[idx]->clearDisplay();
	}

	void SetContrast(int idx, int c) {
		nokia[idx]->setContrast(c);
	}

	int GetScreenCount() {
		return 1;
	}

	Adafruit_GFX * GetScreen(int idx) {
		return nokia[idx];
	}

};
#endif