#include <TM1638.h>

struct ScreenItem {
public:
#ifdef INCLUDE_TM1638
	TM1638* Screen;
#endif
	byte Buttons;
	byte Oldbuttons;
	byte Intensity;
	bool SingleColor;
	ScreenItem() { }
#ifdef INCLUDE_TM1638
	ScreenItem(TM1638* module, bool singleColor) : Screen(module) {
		this->Buttons = 0;
		this->Oldbuttons = 0;
		this->Intensity = 7;
		this->SingleColor = singleColor;
	}
#endif
};

#ifdef INCLUDE_TM1638
TM1638 TM1638_module1(TM1638_DIO, TM1638_CLK, TM1638_STB1, false);
ScreenItem TM1638_screen1(&TM1638_module1, TM1638_SINGLECOLOR1);

TM1638 TM1638_module2(TM1638_DIO, TM1638_CLK, TM1638_STB2, false);
ScreenItem TM1638_screen2(&TM1638_module2, TM1638_SINGLECOLOR2);

TM1638 TM1638_module3(TM1638_DIO, TM1638_CLK, TM1638_STB3, false);
ScreenItem TM1638_screen3(&TM1638_module3, TM1638_SINGLECOLOR3);

TM1638 TM1638_module4(TM1638_DIO, TM1638_CLK, TM1638_STB4, false);
ScreenItem TM1638_screen4(&TM1638_module4, TM1638_SINGLECOLOR4);

TM1638 TM1638_module5(TM1638_DIO, TM1638_CLK, TM1638_STB5, false);
ScreenItem TM1638_screen5(&TM1638_module5, TM1638_SINGLECOLOR5);

TM1638 TM1638_module6(TM1638_DIO, TM1638_CLK, TM1638_STB6, false);
ScreenItem TM1638_screen6(&TM1638_module6, TM1638_SINGLECOLOR6);

// Screen referencing
ScreenItem* TM1638_screens[] = { &TM1638_screen1, &TM1638_screen2, &TM1638_screen3, &TM1638_screen4, &TM1638_screen5, &TM1638_screen6 };

void TM1638_Init() {
	for (int i = 0; i < TM1638_ENABLEDMODULES; i++)
	{
		TM1638_screens[i]->Screen->setupDisplay(true, 7);
		TM1638_screens[i]->Screen->clearDisplay();
	}
}

void TM1638_SetDisplayFromSerial(ScreenItem* screen)
{
	byte displayValues[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	for (int i = 0; i < 8; i++) {
		displayValues[i] = FlowSerialTimedRead();
	}

	screen->Screen->setDisplay(displayValues);

	for (int i = 0; i < 8; i++) {
		char state = (char)FlowSerialTimedRead();

		if (screen->SingleColor) {
			if (state == 'G') {
				state = 'R';
			}
		}

		// Swap led colors if requested
		if (TM1638_SWAPLEDCOLORS == 1) {
			if (state == 'G')
			{
				state = 'R';
			}
			else if (state == 'R')
			{
				state = 'G';
			}
		}

		if (state == 'G') {
			screen->Screen->setLED(TM1638_COLOR_GREEN, i);
		}
		else if (state == 'R') {
			screen->Screen->setLED(TM1638_COLOR_RED, i);
		}
		else if (state == 'Y') {
			screen->Screen->setLED(TM1638_COLOR_RED + TM1638_COLOR_GREEN, i);
		}
		else {
			screen->Screen->setLED(TM1638_COLOR_NONE, i);
		}
	}
}

#endif


