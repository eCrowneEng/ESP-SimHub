#ifndef __SHGAMEPADAXIS_H__
#define __SHGAMEPADAXIS_H__

#include <Arduino.h>
#include <ECrowneJoystick.h>

class SHGamepadAxis {
private:

	int lastAxisValue = -1;
	int axisPin = -1;
	int axisIdx = -1;
	int minimumInputValue;
	int maximumInputValue;
	int samplingRate;
	ECrowneJoystick* joystick;
	float  exponentialFactor;
	float analogReadXXbit(uint8_t analogPin, uint8_t bits_of_precision)
	{
		uint8_t n = bits_of_precision - 10;
		unsigned long oversample_num = 1 << (2 * n);
		uint8_t divisor = 1 << n;
		unsigned long reading_sum = 0;
		unsigned long inner_sum = 0;
		for (unsigned long j = 0; j < oversample_num; j++)
		{
			inner_sum += analogRead(analogPin);
		}
		unsigned int reading = (inner_sum + (unsigned long)divisor / 2UL) >> n;
		reading_sum += reading;
		return  (float)reading_sum;
	}

	void setAxis(int axisIdx, int value) {
		switch (axisIdx)
		{
		case 0:
			joystick->setThrottle(value); break;
		case 1:
			joystick->setAccelerator(value); break;
		case 2:
			joystick->setBrake(value); break;

		default:
			break;
		}

		joystick->sendState();
	}

public:

	SHGamepadAxis(byte axisPin, int axisIdx, int minimumInputValue, int maximumInputValue, int samplingRate, double exponentialFactor = 1) {
		this->axisIdx = axisIdx;
		this->axisPin = axisPin;
		this->minimumInputValue = minimumInputValue;
		this->maximumInputValue = maximumInputValue;
		this->samplingRate = samplingRate > 10 ? 11 : 10;
		this->exponentialFactor = exponentialFactor;
	}

	void SetJoystick(ECrowneJoystick* joystick) {
		this->joystick = joystick;
		read();
	}

	bool read() {
		int pot = analogReadXXbit(axisPin, samplingRate);

		if (lastAxisValue != pot) {
			lastAxisValue = pot;

			int mapped = map(pot, minimumInputValue, maximumInputValue, 0, 1024);
			float mapped2 = min((float)1, max((float)0, (float)mapped / (float)1024));

			if (exponentialFactor != 1) {
				mapped2 = pow(mapped2, 1.0 / (float)exponentialFactor);
			}

			mapped2 = mapped2 * 1024;
			setAxis(axisIdx, mapped2);
		}
	}
};

#endif