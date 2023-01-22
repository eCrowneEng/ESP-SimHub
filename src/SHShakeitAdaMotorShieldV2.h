#ifndef __SHSHAKEITADAMOTORSHIELDV2_H__
#define __SHSHAKEITADAMOTORSHIELDV2_H__

#include <Arduino.h>
#include "SHShakeitBase.h"
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

class SHShakeitAdaMotorShieldV2 : public SHShakeitBase {
private:
	Adafruit_MotorShield Shield1; // Default address, no jumpers
	Adafruit_MotorShield Shield2; // Rightmost jumper closed
	Adafruit_MotorShield Shield3; // Rightmost jumper closed
	Adafruit_DCMotor * AdaMotors[12];
	byte boardsCount = 0;

public:
	uint8_t motorCount() {
		return boardsCount * 4;
	}
	String providerName() {
		return "Adafruit Motor Shield V2";
	}

	void begin(int boardsCount, uint16_t frequency) {
		
		boardsCount = min(3, boardsCount);
		
		this->boardsCount = boardsCount;

		if (boardsCount >= 1) {
			// Default address, no jumpers
			Shield1 = Adafruit_MotorShield(0x60);
			Shield1.begin(frequency);
			AdaMotors[0] = Shield1.getMotor(1);
			AdaMotors[1] = Shield1.getMotor(2);
			AdaMotors[2] = Shield1.getMotor(3);
			AdaMotors[3] = Shield1.getMotor(4);
		}

		if (boardsCount >= 2) {
			// Rightmost jumper closed
			Shield2 = Adafruit_MotorShield(0x61);
			Shield2.begin(frequency);
			AdaMotors[4] = Shield2.getMotor(1);
			AdaMotors[5] = Shield2.getMotor(2);
			AdaMotors[6] = Shield2.getMotor(3);
			AdaMotors[7] = Shield2.getMotor(4);
		}

		if (boardsCount >= 3) {
			// Rightmost jumper closed
			Shield3 = Adafruit_MotorShield(0x62);
			Shield3.begin(frequency);
			AdaMotors[8] = Shield3.getMotor(1);
			AdaMotors[9] = Shield3.getMotor(2);
			AdaMotors[10] = Shield3.getMotor(3);
			AdaMotors[11] = Shield3.getMotor(4);
		}
	}

protected:
	void setMotorOutput(uint8_t motorIdx, uint8_t value) {
		if (value == 0) {
			AdaMotors[motorIdx]->run(RELEASE);
		}
		else {
			AdaMotors[motorIdx]->run(FORWARD);
			AdaMotors[motorIdx]->setSpeed(value);
		}
	}
};

#endif