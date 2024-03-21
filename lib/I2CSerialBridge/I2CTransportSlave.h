#pragma once
#include <Wire.h>
#include "I2CTransport.h"

#ifndef WIRE
#error WIRE must be settled to have a correct custom wire config in your SLAVE config.
#endif

class I2CTransportSlave : public I2CTransport {
	static Stream *outgoingStream;
    public:
	 void setup(Stream *outgoingStream){
		this->outgoingStream=outgoingStream;
		ic2SetupSlave();
	}

	 void loop() {
		
	}
	 void flush() {
		
	}

     static void resendToSerialFromDevice(size_t howManyChars){
 		while (0 <Wire.available()) {
    		char c = WIRE.read();      /* receive byte as a character */
			outgoingStream->write(c);
			}
	};

	 void ic2SetupSlave(){
		WIRE.begin(IC2_ADDRESS);
		WIRE.onReceive(resendToSerialFromDevice); /* register receive event */
	}
};

