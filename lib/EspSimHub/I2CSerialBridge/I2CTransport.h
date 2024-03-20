#include <Wire.h>
#include <HardWareSerial.h>
#include "I2CSerialBridge.h"

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

I2CSerialBridge instance(0x08);

class IC2Transport {
    public:
	static void setup(FullLoopbackStream *outgoingStream){
		instance.setup(outgoingStream);
	}

	static void loop() {
		instance.loop();
	}
	static void flush() {
		instance.flush();
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



#define StreamAvailable WIRE.available
#define FlowSerialFlush WIRE.flush
#define StreamFlush WIRE.flush
#define StreamWrite outgoingStream.write
#define StreamPrint WIRE.print

// size_t I2CWrite(u_int8_t data){
// 	size_t oVal=0;
// 	Wire.beginTransmission(IC2_ADDRESS);
// 	oVal= WIRE.write(data);
// 	endWireTransmission(true);
// 	return oVal;
// }

// size_t I2CWrite(const uint8_t *data, size_t size){
// 	size_t oVal=0;
// 	Wire.beginTransmission(IC2_ADDRESS);
// 	oVal= WIRE.write(data,size);
// 	endWireTransmission(true);
// 	return oVal;
// }



uint8_t endWireTransmission(bool stop){
	uint8_t error=Wire.endTransmission(stop);
	if(error=0){
		Serial.printf("\n Correct wire close \n");
	}
	if(error==1){
		Serial.printf("\n Wire buffer exausted \n");
	}
	if(error==2){
		Serial.printf("\n received NACK on transmit of address.\n");
	}
	if(error==3){
		Serial.printf("received NACK on transmit of data.");
	}
	if(error==4){
		Serial.printf(" other error.");
	}
	if(error==5){
		Serial.printf("timeout");
	}
	Serial.flush();
	return error;

}

bool isSlaveAvailable(){
	Wire.beginTransmission(IC2_ADDRESS);
    uint8_t error = endWireTransmission(true);

	if(error==0){
		Serial.printf("\n Slave device detected at address 8\n");
		return true;
	}
	return false;
}

/** SETUP SERIAL BYPASS IC2 MASTER, USE WHEN THIS DEVICE COMMAND THE SENDING WORKFLOW*/
void ic2SetupMaster(){
	WIRE.begin();
	while(!isSlaveAvailable()){
			Serial.printf("\n Slave device not available, retrying 1 sec later");
			delay(1000);
	};
}


void resendToSerialFromMasterDevice(size_t howManyChars){
	// FlowSerialDebugPrintLn("Received data");
 //int recvBytes=0;

// int buttonId=-1;
 //byte buttonStatus=0;

 while (0 <Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
	// if(recvBytes==3){
	// 	buttonId=c;
	// }
	// if(recvBytes==4){
	// 	buttonStatus=c;
	// }
	// recvBytes++;
   // Serial.write(c);
   //	 Serial.flush();
 //   Serial.print(c);           /* print the character */
	//FlowSerialWrite(c);
  }
	// buttonStatusChanged(buttonId,buttonStatus);
 //Serial.println();             /* to newline */
}

/** SETUP SERIAL BYPASS IC2 SLAVE, USE WHEN THIS DEVICE IS CONNECTED TO SIMHUB*/
void ic2SetupSlave(){
	Wire.begin(IC2_ADDRESS);
	Wire.onReceive(resendToSerialFromMasterDevice); /* register receive event */
}


void ic2SetupSerialBypass(){
 #if IC2_MASTER
	ic2SetupMaster();
 #endif
 #if IC2_SLAVE
	ic2SetupSlave();
 #endif

}

#define FlowSerialBegin [](unsigned long baud) { Serial.printf("Hola mundo");}




