#pragma once
#include <Wire.h>

#include <FullLoopbackStream.h>

class I2CTransport {
	public:
	I2CTransport(){}
	virtual void loop() = 0 ;
	virtual void flush() = 0;
	virtual void setup(Stream *) = 0;
};