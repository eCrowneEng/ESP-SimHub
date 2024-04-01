#ifndef __SHSHAKEITPWMFANS_H__
#define __SHSHAKEITPWMFANS_H__

#include <Arduino.h>
#include "SHShakeitBase.h"
#include <EspSimHubPwm.h>

class SHShakeitPWMFans : public SHShakeitBase {
private:

	byte pins[4];
	byte mins[4];
	byte maxs[4];
	int relays[4];
	unsigned long offSince[4];
	unsigned long offDelay[4] = { 2000,2000,2000,2000 };
	byte reverseRelayLogic[4] = { 0,0,0,0 };
	byte enabledOutputs;
#ifdef ESP32
	ledc_components* outputs;
#endif
public:
	uint8_t motorCount() {
		return enabledOutputs;
	}

	String providerName() {
		return "PWMFan";
	}

	void safetyStop() override {
		SHShakeitBase::safetyStop();

		for (int i = 0; i < enabledOutputs; i++) {
			if (relays[i] > 0) {
				SetRelayState(i, false);
			}
		}
	}

	void begin(byte pEnabledOutputs, byte pPin01, byte pPin02, byte	pPin03, byte pPin04) {
		pins[0] = pPin01;
		pins[1] = pPin02;
		pins[2] = pPin03;
		pins[3] = pPin04;

		enabledOutputs = pEnabledOutputs;
		#ifdef ESP32
		ledc_components* components = new ledc_components[pEnabledOutputs];
		for (int i = 0; i < pEnabledOutputs; i++) {
			components[i].pin = pins[i];
		}
		createPwmForDutyCycleControl(components, enabledOutputs, 25000, 10);
		outputs = components;
		#endif
		#ifdef ESP8266
		for (int i = 0; i < pEnabledOutputs; i++) {
			pinMode(pins[i], OUTPUT);
			analogWrite(pins[i], 0);
		}
		analogWriteFreq(25000); // 25khz
        // set duty cycle range from 0 to 1023, per requirement of simhub params (10 bit)
        analogWriteRange(1023);
		#endif
	}

	void setMin(byte pMin01, byte pMin02, byte	pMin03, byte pMin04) {
		mins[0] = pMin01;
		mins[1] = pMin02;
		mins[2] = pMin03;
		mins[3] = pMin04;
	}

	void setMax(byte pMax01, byte pMax02, byte	pMax03, byte pMax04) {
		maxs[0] = pMax01;
		maxs[1] = pMax02;
		maxs[2] = pMax03;
		maxs[3] = pMax04;
	}

	void setRelays(
		int r01, int r02, int r03, int r04,
		int d01, int d02, int d03, int d04,
		bool l01, bool l02, bool l03, bool l04) {
		relays[0] = r01;
		relays[1] = r02;
		relays[2] = r03;
		relays[3] = r04;

		offDelay[0] = d01;
		offDelay[1] = d02;
		offDelay[2] = d03;
		offDelay[3] = d04;

		reverseRelayLogic[0] = l01;
		reverseRelayLogic[1] = l02;
		reverseRelayLogic[2] = l03;
		reverseRelayLogic[3] = l04;

		for (int i = 0; i < enabledOutputs; i++) {
			if (relays[i] > 0) {
				pinMode(relays[i], OUTPUT);
				SetRelayState(i, false);
			}
			offSince[i] = 0;
		}
	}

protected:

	void SetRelayState(int relayIdx, bool state) {
		int relayPin = relays[relayIdx];
		if (relayPin > 0) {

			if (!state) {
				digitalWrite(relayPin, reverseRelayLogic[relayIdx] ? HIGH : LOW);
			}
			else {
				digitalWrite(relayPin, reverseRelayLogic[relayIdx] ? LOW : HIGH);
			}
		}
	}

	void setMotorOutput(uint8_t motorIdx, uint8_t value) {
		double value2 = value;
		if (value2 < mins[motorIdx]) {
			value2 = 0;
		}
		else {
			value2 = (value2) / 255.0 * (double)maxs[motorIdx];
		}
		uint32_t duty = (value2 / 255.0) * 1023.0;
		#ifdef ESP8266
		analogWrite(pins[motorIdx], (int)(((float)value2 / 255.0) * 1023.0));
		#endif
		#ifdef ESP32
		ledc_components components = outputs[motorIdx];
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

		if (relays[motorIdx] > 0) {
			if (value2 > 0) {
				SetRelayState(motorIdx, true);
				offSince[motorIdx] = 0;
			}
			else {
				if (offSince[motorIdx] == 0) {
					offSince[motorIdx] = millis();
				}

				if ((millis() - offSince[motorIdx]) > offDelay[motorIdx]) {
					SetRelayState(motorIdx, false);
					offSince[motorIdx] = 0;
				}
			}
		}
	}
};

#endif