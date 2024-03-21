#pragma once
#include <Wire.h>
#include <HardWareSerial.h>
#include <FullLoopbackStream.h>

#ifndef WIRE
    # define WIRE Wire;
#endif

#ifndef IC2_MASTER && IC2_SLAVE
    #define IC2_MASTER
#endif

#ifndef IC2_ADDRESS
    #define IC2_ADDRESS 0x08
#endif
#define StreamRead WIRE.read
//  #if IC2_MASTER
//     ic2SetupMaster();
// #endif
// #if IC2_SLAVE
//     ic2SetupSlave();
// #endif

class I2CTransport {
	public:
	I2CTransport(){}
	virtual void loop() = 0 ;
	virtual void flush() = 0;
	virtual void setup(Stream *) = 0;
};