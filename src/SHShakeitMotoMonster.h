#ifndef __SHSHAKEITMOTOMONSTER_H__
#define __SHSHAKEITMOTOMONSTER_H__

#include <Arduino.h>
#include "SHShakeitBase.h"
#include "SHMotoMonster.h"

class SHShakeitMotoMonster : public SHShakeitBase {
private:
	bool reverseDirectionEnabled;

public:
	uint8_t motorCount() {
		return 2;
	}
	String providerName() {
		return "MotoMonster";
	}

	void begin(bool reverseDirection) {
		reverseDirectionEnabled = reverseDirection;
		setupSHMotoMonster();
	}

protected:
	void setMotorOutput(uint8_t motorIdx, uint8_t value) {
		if (value > 0) {
			SHMM_motorGo(motorIdx, reverseDirectionEnabled ? SH_CCW : SH_CW, value);
		}
		else {
			SHMM_motorOff(motorIdx);
		}
	}
};

#endif