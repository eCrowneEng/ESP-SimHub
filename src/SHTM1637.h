#include "TM1637Display.h"

TM1637Display TM1637_module1(TM1637_CLK1, TM1637_DIO1);
TM1637Display TM1637_module2(TM1637_CLK2, TM1637_DIO2);
TM1637Display TM1637_module3(TM1637_CLK3, TM1637_DIO3);
TM1637Display TM1637_module4(TM1637_CLK4, TM1637_DIO4);
TM1637Display TM1637_module5(TM1637_CLK5, TM1637_DIO5);
TM1637Display TM1637_module6(TM1637_CLK6, TM1637_DIO6);
TM1637Display TM1637_module7(TM1637_CLK7, TM1637_DIO7);
TM1637Display TM1637_module8(TM1637_CLK8, TM1637_DIO8);

TM1637Display* TM1637_screens[] = { &TM1637_module1, &TM1637_module2, &TM1637_module3, &TM1637_module4, &TM1637_module5, &TM1637_module6, &TM1637_module7, &TM1637_module8 };

uint8_t TM1637_screens_digits[4]= { 0x00, 0x00, 0x00, 0x00 };

void TM1637_SetDisplayFromSerial(TM1637Display* screen)
{
	

	for (int i = 0; i < 4; i++) {
		TM1637_screens_digits[i] = (uint8_t)FlowSerialTimedRead();
		
		
		
	}
	screen->setSegments(TM1637_screens_digits, 4, 0);
	//screen->setSegments(i, );

	// Skip 4 remaining chars
	for (int i = 0; i < 4; i++) {
		FlowSerialTimedRead();
	}
}

void TM1637_Init() {
	for (int i = 0; i < TM1637_ENABLEDMODULES; i++) {
		TM1637_screens[i]->init();
		TM1637_screens[i]->setBrightness(4);
		TM1637_screens[i]->clear();
	}
}