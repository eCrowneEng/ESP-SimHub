#ifndef __SHSHAKEITBASE_H__
#define __SHSHAKEITBASE_H__

#include <Arduino.h>
#define SHShakeitBaseSafetyDelay 1000
class SHShakeitBase {
private:
	unsigned long lastRead = 0;

public:
	virtual uint8_t motorCount();
	virtual String providerName();

	virtual void safetyStop() {
		uint8_t motorcount = motorCount();
		for (int m = 0; m < motorcount; m++) {
			setMotorOutput(m, 0);
		}
	}

	void safetyCheck() {
		if (millis() - lastRead > SHShakeitBaseSafetyDelay && lastRead > 0) {
			safetyStop();
			lastRead = 0;
		}
	}

	void read() {
		uint8_t motorcount = motorCount();

		for (int motorIdx = 0; motorIdx < motorcount; motorIdx++) {
			int value = FlowSerialTimedRead();
			if (value != -1) {
				setMotorOutput(motorIdx, value);
			}
			else {
				return;
			}
		}
		lastRead = millis();
	}

protected:
	virtual void setMotorOutput(uint8_t motorIdx, uint8_t value);
};

#endif