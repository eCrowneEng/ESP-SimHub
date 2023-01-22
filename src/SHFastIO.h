#ifndef __SHFASTIO_H__
#define __SHFASTIO_H__
#include <Arduino.h>

class FastDigitalPin {
private:
	uint8_t bit = 0;
	uint8_t port = 0;
	bool pinIsValid = false;
public:

	bool isValid() {
		return pinIsValid;
	}
	void begin(int pin) {
		if (pin < 0) {
			pinIsValid = false;
		}
		else {
			pinIsValid = true;
			bit = digitalPinToBitMask(pin);
			port = digitalPinToPort(pin);
		}
	}

	int digitalRead()
	{
		if (*portInputRegister(port) & bit) return HIGH;
		return LOW;
	}
};

#endif