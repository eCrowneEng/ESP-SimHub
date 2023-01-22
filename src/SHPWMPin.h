#ifndef __SHPWMPIN_H__
#define __SHPWMPIN_H__
#include <Arduino.h>

class SHPWMPin {
private:
	uint8_t p;
public:

	SHPWMPin(int pin, bool tunePwm) : SHPWMPin(pin) {
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
		if (tunePwm) {
			setPwmFrequency(pin, 1);
		}
#endif
	}

	SHPWMPin(int pin) {
		p = pin;
		pinMode(pin, OUTPUT);
	}

	SetValue(int value) {
		analogWrite(p, value);
	}

	void readFromString()
	{
		analogWrite(p, FlowSerialReadStringUntil('\n').toInt());
	}
};

#endif