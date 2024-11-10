#pragma once
#include <Wire.h>
#include "I2CTransport.h"

#define StreamWrite

#ifndef WIRE
#error WIRE must be settled to have a correct custom wire config in your SLAVE config.
#endif

#ifndef I2C_ADDRESS
#error I2C_ADDRESS must be settled to have a correct custom wire config in your SLAVE config.
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
		WIRE.begin(I2C_ADDRESS);
		/**
		 * BECAUSE THERE IS NO JOYSTICK SUPPORT IN THE ESP IMPL DEFAULT IMPL FOR ESP IS THIS IMPL TO RESEND ALL THE FLOW
		 * TO THE TRANSPORT STREAM WHICH IS CONFIGURED FOR THE DEVICE. 
		*/
		WIRE.onReceive(resendToSerialFromDevice); /* register receive event */
	}
};

