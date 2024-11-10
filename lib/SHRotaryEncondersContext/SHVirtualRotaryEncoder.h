#pragma once
#ifndef __SHVIRTUALROTARYENCODER_H__
#define __SHVIRTUALROTARYENCODER_H__

#include <Arduino.h>
#include <Stream.h>
#include <SHRotaryEncodersContext.h>
#include <SHRotaryEncoder.h>

class SHVirtualRotaryEncoder : public SHRotaryEncoder {
private:
    SHRotaryEncoderContext* context;

public:

     SHVirtualRotaryEncoder(SHRotaryEncoderContext* context){
         this->context=context;
     }

    void begin(uint8_t outputAPin, uint8_t outputBPin, int buttonPin, bool reverse, bool enablePullup, byte encoderid, bool half, SHRotaryEncoderPositionChanged changedcallback) override{
        this->id=encoderid;
        this->inputLastState = 0;
		this->buttonLastState = enablePullup? HIGH: LOW;
        this->halfSteps=half;
        this->positionChangedCallback=changedcallback;
        context->updateContext(id,0,255);
    }

    uint8_t getDirection(uint8_t delay, unsigned long referenceTime) override{
        // Serial.print(" DirecionLastCHange:");
        // Serial.print(this->context->getDirection(this->id));
        // Serial.print(" | ");
        // Serial.print(" ReferenceTime:");
        // Serial.print(referenceTime);
        // Serial.print(" | ");
        // Serial.print(" getPositionLastChanged:");
        // Serial.print(context->getPositionLastChanged(this->id));
        // Serial.print(" | ");
        // Serial.print(" Diff:");
        // Serial.print(referenceTime - context->getPositionLastChanged(this->id));
        // Serial.print(" | \n");

		if (directionLastChange != 255 && (referenceTime - context->getPositionLastChanged(this->id)) < delay) {
		   // this->context->updateContext(this->id,counter,directionLastChange);
			return directionLastChange;
		}
	   // this->context->updateContext(this->id,counter,255);
		return 255;
    }

    uint8_t getPressed() override{
		return !buttonLastState;
	}

    void read() override{
        
        this->direction=this->context->getDirection(this->id);
        this->counter=this->context->getPosition(this->id);

        // Serial.print(" SHVirtualRotaryEncoder::read:direction:");
        // Serial.print(this->context->getDirection(this->id));
        // Serial.print(" \n");

        if (direction == 0) {
			counter++;
			positionChangedCallback(id, counter, direction);
			//positionLastChanged = millis();
			directionLastChange = 0;
		}
		else if (direction == 1) {
			counter--;
			positionChangedCallback(id, counter, direction);
			//positionLastChanged = millis();
			directionLastChange = 1;
		}

        this->buttonLastState=context->getButtonState(id);

       //SHRotaryEncoder::read();
    }



};


#endif