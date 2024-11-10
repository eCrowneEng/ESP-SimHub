#pragma once
#include <Arduino.h>
#include <SHRotaryEncoder.h>

class SHRotaryEncoderContext {
private:
     byte direction[8];
     int  position[8];
     unsigned long positionLastChanged[8];
     bool buttonPressed[8];
    

public:
    void init(){

    }

    void updateContext(int encoderId, int pos, byte dir){
        // char sbuf[150];
	    // sprintf(sbuf,"\nSHRotaryEncoderContext::updateContext(%d,%d,%d);\n",encoderId,pos,dir);
	    // Serial.print(sbuf);
        if(dir==0xD7){
            this->buttonPressed[encoderId-1]=pos == 1? 0 : 1;
            Serial.print(this->buttonPressed[encoderId-1]);
	    }else{
        this->direction[encoderId-1]=dir;
        this->position[encoderId-1]=pos;
        this->positionLastChanged[encoderId-1]=millis();
        }

        // sprintf(sbuf,"\nSHRotaryEncoderContext::updateContext::direction:%d",this->direction[encoderId-1]);
	    // Serial.print(sbuf);
        // sprintf(sbuf,"\nSHRotaryEncoderContext::updateContext::position:%d",this->position[encoderId-1]);
	    // Serial.print(sbuf);
        // sprintf(sbuf,"\nSHRotaryEncoderContext::updateContext::positionLastChanged:%lu",this->positionLastChanged[encoderId-1]);
	    // Serial.print(sbuf);
        // Serial.println("");
        
    }

    int getPosition(int encoderId){
        return this->position[encoderId-1];
    }

    byte getDirection(int encoderId){
        return this->direction[encoderId-1];
    }

    unsigned long getPositionLastChanged(int encoderId){
        return this->positionLastChanged[encoderId-1];
    }

    bool getButtonState(int encoderId){
        return this->buttonPressed[encoderId-1];
    }

    void logDirection(){
        char sbuf[150];
        for(int i=0;i<8;i++){
            sprintf(sbuf+strlen(sbuf)," %d,",this->direction[i]);
        }
      //  Serial.print("\nSHRotaryEncoderContext::direction: ");
        Serial.print(sbuf);
        Serial.println("");

    }

    void logPosition(){
          char sbuf[150];
        for(int i=0;i<8;i++){
            sprintf(sbuf+strlen(sbuf)," %d,",this->position[i]);
        }
     //   Serial.print("\nSHRotaryEncoderContext::position: ");
        Serial.print(sbuf);
        Serial.println("");
    }

};