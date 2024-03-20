#pragma once
#include <Wire.h>
#include "I2CTransport.h"






class IC2TransportSlave : public IC2Transport {
    public:
	 void setup(FullLoopbackStream *outgoingStream){
		ic2SetupSlave();
	}

	 void loop() {
		
	}
	 void flush() {
		
	}

     static void resendToSerialFromDevice(size_t howManyChars){
 		while (0 <Wire.available()) {
    		char c = WIRE.read();      /* receive byte as a character */
			Serial.write(c);
			}
	};

	 void ic2SetupSlave(){
		WIRE.begin(IC2_ADDRESS);
		WIRE.onReceive(resendToSerialFromDevice); /* register receive event */
	}
};

