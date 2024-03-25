#pragma once
#include <Wire.h>
#include "I2CTransport.h"

#define StreamWrite

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
		byte bffer[Wire.available()];
		int bfferIdx=0;
 		while (0 <Wire.available()) {
			char c = WIRE.read();      /* receive byte as a character */
			bffer[bfferIdx]=c;
			bfferIdx++;
			}
		StreamWrite(&bffer,sizeof(bffer));
	};

	 void ic2SetupSlave(){
		WIRE.begin(IC2_ADDRESS);
		WIRE.onReceive(resendToSerialFromDevice); /* register receive event */
	}
};

