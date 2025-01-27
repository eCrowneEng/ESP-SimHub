#ifndef __SHBUTTON_H__
#define __SHBUTTON_H__

#include <Arduino.h>
#include "SHFastIO.h"

typedef void(*SHButtonChanged) (int, byte);

class SHButton {
private:

	uint8_t buttonState;
	byte buttonLastState = -1;
	bool vccToPinWiring;
	unsigned long buttonLastStateChanged;
	byte id;
	byte buttonPin;
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
		this->vccToPinWiring = vccToPinWiring;
		this->logicMode = logicMode;
		this->buttonPin = buttonPin;

		if (buttonPin >= 0) {
			if (!vccToPinWiring) {
				pinMode(buttonPin, INPUT_PULLUP);
			}
			else {
				pinMode(buttonPin, INPUT);
			}
		}
		id = buttonid;
		buttonLastState = getState(digitalRead(buttonPin));
		shButtonChangedCallback = changedcallback;
		
		if (buttonLastState)
			shButtonChangedCallback(id, buttonLastState);
	}

	uint8_t getPressed() {
		return !buttonLastState;
	}

	void read() {
		buttonState = getState(digitalRead(buttonPin));
		if (buttonState != buttonLastState && buttonLastStateChanged - millis() > 50) {
			shButtonChangedCallback(id, buttonState);
			buttonLastState = buttonState;
			buttonLastStateChanged = millis();
		}
	}
};

#endif