#ifndef __SHSHAKEITDKMOTORSHIELD_H__
#define __SHSHAKEITDKMOTORSHIELD_H__

#include <Arduino.h>
#include "SHShakeitBase.h"
#include <AFMotor.h>

// Goes mad when instantiated inside the class
AF_DCMotor dkmotor1(1, MOTOR12_64KHZ);
AF_DCMotor dkmotor2(2, MOTOR12_64KHZ);
AF_DCMotor dkmotor3(3, MOTOR34_64KHZ);
AF_DCMotor dkmotor4(4, MOTOR34_64KHZ);
AF_DCMotor * DKMotors[] = { &dkmotor1, &dkmotor2, &dkmotor3, &dkmotor4 };

class SHShakeitDKMotorShield : public SHShakeitBase {
private:

	bool reverseDirectionEnabled;
	
public:
	uint8_t motorCount() {
		return 4;
	}
	
	String providerName() {
		return "DK Motor Shield";
	}

	void begin(bool useHummingReducing) {
		
		
		if (useHummingReducing) {
			setPwmFrequency(3, 1);
			setPwmFrequency(5, 1);
			setPwmFrequency(6, 1); 
		}

		DKMotors[0]->run(FORWARD);
		DKMotors[1]->run(FORWARD);
		DKMotors[2]->run(FORWARD);
		DKMotors[3]->run(FORWARD);
	}

protected:
	void setMotorOutput(uint8_t motorIdx, uint8_t value) {
		if (value > 0) {
			DKMotors[motorIdx]->run(FORWARD);
			DKMotors[motorIdx]->setSpeed(value);
		}
		else {
			DKMotors[motorIdx]->run(RELEASE);
		}
	}
};

#endif