
#pragma once
#include "I2CTransport.h"
#include "I2CSerialBridge.h"

#ifndef WIRE
#error WIRE must be settled to have a correct custom wire config in your MASTER config.
#endif

#if IC2_MASTER
I2CSerialBridge instance(IC2_ADDRESS);

class I2CTransportMaster : public I2CTransport {
    public:

    I2CTransportMaster(){}
	 void setup(Stream *outgoingStream){
		ic2SetupSerialBypass();
		instance.setup((FullLoopbackStream*) outgoingStream);
	}
	 void loop() {
		instance.loop();
	}
	 void flush() {
		instance.flush();
	}


	 void ic2SetupSerialBypass(){
         #if IC2_SERIAL_BYPASS_DEBUG
            Serial.print("\n Setting up the master slave connection with slave at ");
            Serial.print(IC2_ADDRESS);
            Serial.println("\n");
        #endif
			#if IC2_MASTER
				instance.ic2SetupMaster();
			#endif
		
	}

    // static size_t write(const uint8_t *buffer, size_t size){
	// 	Wire.beginTransmission(IC2_ADDRESS);
	// 	Wire.write(buffer,size);
	// 	return Wire.endTransmission();
	// }
    // static size_t write(const char *buffer, size_t size){
	// 	Wire.beginTransmission(IC2_ADDRESS);
	// 	Wire.write(buffer,size);
	// 	return Wire.endTransmission();
	// }
    // static size_t write(const char *str){
	// 	Wire.beginTransmission(IC2_ADDRESS);
	// 	Wire.write(str);
	// 	return Wire.endTransmission();
	// }
	// static size_t write(uint8_t data){
	// 	Wire.beginTransmission(IC2_ADDRESS);
	// 	Wire.write(data);
	// 	return Wire.endTransmission();
	// }

};



// #define StreamAvailable WIRE.available
// #define FlowSerialFlush WIRE.flush
// #define StreamFlush WIRE.flush
// #define StreamWrite outgoingStream.write
// #define StreamPrint WIRE.print
// /** SETUP SERIAL BYPASS IC2 SLAVE, USE WHEN THIS DEVICE IS CONNECTED TO SIMHUB*/
// #define FlowSerialBegin [](unsigned long baud) { Serial.printf("Hola mundo");}
#endif