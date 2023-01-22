#ifndef __SHBUTTON_H__
#define __SHBUTTON_H__

#include <Arduino.h>
#include "SHFastIO.h"

typedef void(*SHButtonChanged) (int, byte);

class SHButton {
private:

	FastDigitalPin button;
	uint8_t buttonState;
	int buttonLastState = -1;
	bool vccToPinWiring;
	unsigned long buttonLastStateChanged;
	byte id;
	SHButtonChanged shButtonChangedCallback;
	int logicMode;

	int getState(int value) {
		int res = 0;
		if (!vccToPinWiring) {
			res = value == HIGH ? 0 : 1;
		}
		else {
			res = value == HIGH ? 1 : 0;
		}
		if (logicMode == 1) {
			res = res ? 0 : 1;
		}
		return res;
	}

public:

	void begin(byte buttonid, uint8_t buttonPin, SHButtonChanged changedcallback, bool vccToPinWiring, int logicMode) {
		button.begin(buttonPin);
		this->vccToPinWiring = vccToPinWiring;
		this->logicMode = logicMode;

		if (buttonPin > 0) {
			if (!vccToPinWiring) {
				pinMode(buttonPin, INPUT_PULLUP);
			}
			else {
				pinMode(buttonPin, INPUT);
			}
		}
		id = buttonid;
		buttonLastState = getState(button.digitalRead());
		shButtonChangedCallback = changedcallback;
		
		if (buttonLastState)
			shButtonChangedCallback(id, buttonLastState);
	}

	uint8_t getPressed() {
		return !buttonLastState;
	}

	void read() {
		buttonState = getState(button.digitalRead());
		if (buttonState != buttonLastState && buttonLastStateChanged - millis() > 50) {
			shButtonChangedCallback(id, buttonState);
			buttonLastState = buttonState;
			buttonLastStateChanged = millis();
		}
	}
};

#endif