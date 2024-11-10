
#pragma once
#include "I2CTransport.h"
#include "I2CSerialBridge.h"

#ifndef WIRE
#error WIRE must be settled to have a correct custom wire config in your MASTER config.
#endif

#if I2C_BYPASS_MASTER
I2CSerialBridge instance(I2C_ADDRESS);

class I2CTransportMaster : public I2CTransport {
    public:

    I2CTransportMaster(){}
	 void setup(Stream *outgoingStream){
		i2cSetupSerialBypass();
		instance.setup((FullLoopbackStream*) outgoingStream);
	}
	 void loop() {
		instance.loop();
	}
	 void flush() {
		instance.flush();
	}


	 void i2cSetupSerialBypass(){
         #if I2C_SERIAL_BYPASS_DEBUG
            Serial.print("\n Setting up the master slave connection with slave at ");
            Serial.print(I2C_ADDRESS);
            Serial.println("\n");
        #endif
			#if I2C_BYPASS_MASTER
				instance.i2cSetupMaster();
			#endif
		
	}
};
#endif