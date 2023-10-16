#ifndef __SHSHAKEITDUALVNH5019_H__
#define __SHSHAKEITDUALVNH5019_H__

#include <Arduino.h>
#include "SHShakeitBase.h"
#include "DualVNH5019MotorShield.h"

DualVNH5019MotorShield dualVNH5019MotorShield;

class SHShakeitDualVNH5019 : public SHShakeitBase {
private:
	bool _reverseDirection = false;
public:
	uint8_t motorCount() {
		return 2;
	}

	String providerName() {
		return "Dual VNH5019";
	}

	void begin(bool reverseDirection) {
		dualVNH5019MotorShield.init();
		_reverseDirection = reverseDirection;
	}

protected:
	void setMotorOutput(uint8_t motorIdx, uint8_t value) {
		int speed = (int)((float)((float)value * 400) / (float)(255));

		if (_reverseDirection) {
			speed *= -1;
		}
		if (motorIdx == 0) {
			dualVNH5019MotorShield.setM1Speed(speed);
		}
		if (motorIdx == 1) {
			dualVNH5019MotorShield.setM2Speed(speed);
		}
	}
};

#endif