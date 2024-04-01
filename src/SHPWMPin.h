#ifndef __SHPWMPIN_H__
#define __SHPWMPIN_H__
#include <Arduino.h>
#include <EspSimHubPwm.h>

#ifdef ESP32
#include "driver/ledc.h"
#endif

/**
 * TODO: if all of these pins use the same frequency, we can create a single timer and maximize resources, otherwise
 *  this will create a timer per pin, depleting the 4 that exist very quickly if using many pwm outputs
 */
class SHPWMPin {
private:
#ifdef ESP8266
	int p = -1;
#endif
#ifdef ESP32
	ledc_components output;
	char resolution = 8;
#endif

public:
	SHPWMPin(int pin) {
		#ifdef ESP32
		ledc_components* components = new ledc_components[1];
		components[0].pin = pin;
		// we should never set the frequency to 0, because that breaks ledc, instead, we set duty cycle to 0 to disable an output (see below)
		createPwmForFrequencyControl(components, 1, 100, resolution, pow(2, resolution) / 2);
		output = components[0];
		auto speedMode = output.channel.speed_mode;
		auto channel = output.channel.channel;
		ledc_set_duty(speedMode, channel, 0);
		#endif

		#ifdef ESP8266
		this->p = pin;
		analogWriteRange(1023);
		analogWrite(pin, 0);
		#endif
	}

	void SetValue(int value) {
		#ifdef ESP8266
		analogWrite(this->p, value);
		#endif

		#ifdef ESP32
		ledc_components components = output;
		ledc_mode_t speedMode = components.channel.speed_mode;
		ledc_channel_t channel = components.channel.channel;
		ledc_set_duty(
			speedMode, 
			channel,
			value
		);
		ledc_update_duty(
			speedMode, 
			channel
		);
		#endif
	}

	void readFromString()
	{
		auto duty = FlowSerialReadStringUntil('\n').toInt();
		#ifdef ESP8266
		analogWrite(this->p, duty);
		#endif

		#ifdef ESP32
		ledc_components components = output;
		ledc_mode_t speedMode = components.channel.speed_mode;
		ledc_channel_t channel = components.channel.channel;
		ledc_set_duty(
			speedMode, 
			channel,
			duty
		);
		ledc_update_duty(
			speedMode, 
			channel
		);
		#endif
	}
};

#endif