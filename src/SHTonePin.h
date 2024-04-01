#ifndef __SHTONEPIN_H__
#define __SHTONEPIN_H__
#include <Arduino.h>
#include <EspSimHubPwm.h>

#ifdef ESP32
#include "driver/ledc.h"
#endif

class SHTonePin {
private:
#ifdef ESP8266
	int pin = -1;
#endif
#ifdef ESP32
	ledc_components output;
	char resolution = 14;
#endif
public:
	void begin(int pin) {
		#ifdef ESP32
		ledc_components* components = new ledc_components[1];
		components[0].pin = pin;
		// we should never set the frequency to 0, because that breaks ledc, instead, we set duty cycle to 0 to disable an output
		createPwmForFrequencyControl(components, 1, 100, resolution, pow(2, resolution) / 2);
		output = components[0];
		auto speedMode = output.channel.speed_mode;
		auto channel = output.channel.channel;
		ledc_set_duty(speedMode, channel, 0);
		#endif
		#ifdef ESP8266
		this->pin = pin;
		analogWriteRange(1023);
		analogWrite(pin, 0);
		#endif
	}

	void readFromString()
	{
		int freq = FlowSerialReadStringUntil('\n').toInt();
		
		#ifdef ESP32
		ledc_components components = output;
		auto speedMode = components.channel.speed_mode;
		auto timer = components.timer.timer_num;
		auto channel = components.channel.channel;
		if (freq < 1) {
			ledc_set_duty(speedMode, channel, 0);
		} else {
			ledc_set_freq(speedMode, timer, freq);
			ledc_set_duty(speedMode, channel, pow(2, resolution) / 2);
		}
		int freqRes = ledc_get_freq(speedMode, timer);
		#endif
		#ifdef ESP8266
		if (freq < 1) {
			analogWrite(pin, 0);
		} else {
			analogWriteFrequency(freq);
			analogWrite(pin, 255.0 / 2);
		}
		#endif
	}
};

#endif
