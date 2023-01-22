#include "TM1637_6D.h"

TM1637_6D TM1637_6D_module1(TM1637_6D_CLK1, TM1637_6D_DIO1);
TM1637_6D TM1637_6D_module2(TM1637_6D_CLK2, TM1637_6D_DIO2);
TM1637_6D TM1637_6D_module3(TM1637_6D_CLK3, TM1637_6D_DIO3);
TM1637_6D TM1637_6D_module4(TM1637_6D_CLK4, TM1637_6D_DIO4);
TM1637_6D TM1637_6D_module5(TM1637_6D_CLK5, TM1637_6D_DIO5);
TM1637_6D TM1637_6D_module6(TM1637_6D_CLK6, TM1637_6D_DIO6);
TM1637_6D TM1637_6D_module7(TM1637_6D_CLK7, TM1637_6D_DIO7);
TM1637_6D TM1637_6D_module8(TM1637_6D_CLK8, TM1637_6D_DIO8);

TM1637_6D* TM1637_6D_screens[] = { &TM1637_6D_module1, &TM1637_6D_module2, &TM1637_6D_module3, &TM1637_6D_module4, &TM1637_6D_module5, &TM1637_6D_module6, &TM1637_6D_module7, &TM1637_6D_module8 };

uint8_t TM1637_6D_screens_digits[6]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t TM1637_6D_points_digits[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void TM1637_6D_SetDisplayFromSerial(TM1637_6D* screen)
{
	
	for (int i = 0; i < 6; i++) {
		TM1637_6D_screens_digits[i] = (uint8_t)FlowSerialTimedRead();
	}
	screen->displayRaw(TM1637_6D_screens_digits);
	
	// Skip 2 remaining chars
	for (int i = 0; i < 2; i++) {
		FlowSerialTimedRead();
	}
}

void TM1637_6D_Init() {
	for (int i = 0; i < TM1637_6D_ENABLEDMODULES; i++) {
		TM1637_6D_screens[i]->init();
		//TM1637_6D_screens[i]->set(4);
		TM1637_6D_screens[i]->clearDisplay();
	}
}