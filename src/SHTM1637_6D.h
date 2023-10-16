#include "TM1637Display.h"

TM1637Display TM1637_6D_module1(TM1637_6D_CLK1, TM1637_6D_DIO1);
TM1637Display TM1637_6D_module2(TM1637_6D_CLK2, TM1637_6D_DIO2);
TM1637Display TM1637_6D_module3(TM1637_6D_CLK3, TM1637_6D_DIO3);
TM1637Display TM1637_6D_module4(TM1637_6D_CLK4, TM1637_6D_DIO4);
TM1637Display TM1637_6D_module5(TM1637_6D_CLK5, TM1637_6D_DIO5);
TM1637Display TM1637_6D_module6(TM1637_6D_CLK6, TM1637_6D_DIO6);
TM1637Display TM1637_6D_module7(TM1637_6D_CLK7, TM1637_6D_DIO7);
TM1637Display TM1637_6D_module8(TM1637_6D_CLK8, TM1637_6D_DIO8);

TM1637Display* TM1637_6D_screens[] = { &TM1637_6D_module1, &TM1637_6D_module2, &TM1637_6D_module3, &TM1637_6D_module4, &TM1637_6D_module5, &TM1637_6D_module6, &TM1637_6D_module7, &TM1637_6D_module8 };

uint8_t TM1637_6D_screens_digits[6]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t TM1637_6D_points_digits[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void TM1637_6D_SetDisplayFromSerial(TM1637Display* screen)
{
#pragma message("This code hasnt been tested, but it was adapted to use the same library we installed previously");
#pragma message("Let us know if it works in a GitHub issue or a message in the Discord");
	for (int i = 0; i < 6; i++) {
		TM1637_6D_screens_digits[i] = (uint8_t)FlowSerialTimedRead();
		screen->encodeDigit(TM1637_6D_screens_digits[i]);
	}
	
	// Skip 2 remaining chars
	for (int i = 0; i < 2; i++) {
		FlowSerialTimedRead();
	}
}

void TM1637_6D_Init() {
	for (int i = 0; i < TM1637_6D_ENABLEDMODULES; i++) {
		TM1637_6D_screens[i]->clear();
	}
}