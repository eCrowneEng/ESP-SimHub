#ifndef __SHTONEPIN_H__
#define __SHTONEPIN_H__
#include <Arduino.h>
#include <Tone.h>

class SHTonePin {
private:
	Tone tone;
public:

	void begin(int pin) {
		tone = Tone();
		tone.begin(pin);
	}

	void readFromString()
	{
		int level = FlowSerialReadStringUntil('\n').toInt();
		if (level < 1)
			tone.stop();
		else
			tone.play(level);
	}
};

#endif