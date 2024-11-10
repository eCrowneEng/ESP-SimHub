#pragma once
#include "I2CTransportMaster.h"
#include "I2CTransportSlave.h"

/***
 * "This Manager is responsible for configuring the behavior based on the master-slave role for I2C communication."
*/
class I2CTransportManager  {
    public:
    #if I2C_BYPASS_MASTER
       static I2CTransportMaster tm;
    #endif
       static I2CTransportSlave ts;

    static void setup(FullLoopbackStream *outgoingStream){
        #if I2C_BYPASS_MASTER
            #if I2C_SERIAL_BYPASS_DEBUG
                Serial.print("\nSetup as Master\n");
                Serial.flush();
            #endif
        tm.setup(outgoingStream);
        #endif

        #if I2C_BYPASS_SLAVE
        Serial.print("Setup as Slave");
        Serial.flush();
        ts.setup(outgoingStream);
        #endif
    }
    static void loop(){
        #if I2C_BYPASS_MASTER
        tm.loop();
        #endif

        #if I2C_BYPASS_SLAVE
        ts.loop();
        #endif
    }

    static void flush(){
        #if I2C_BYPASS_MASTER
            tm.flush();
        #endif

        #if I2C_BYPASS_SLAVE
            ts.flush();
        #endif
    }
};

#if I2C_BYPASS_MASTER
#define StreamRead outgoingStream.read
#define StreamAvailable outgoingStream.available
#define FlowSerialFlush Serial.flush
#define StreamFlush I2CTransportManager::flush
#define StreamWrite outgoingStream.write
//#define StreamWrite WIRE.write
#define StreamPrint outgoingStream.print
/** SETUP SERIAL BYPASS I2C SLAVE, USE WHEN THIS DEVICE IS CONNECTED TO SIMHUB*/
#define FlowSerialBegin [](unsigned long baud) { Serial.print("Hola mundo");}
#endif
 #if I2C_BYPASS_SLAVE
 #define StreamRead Serial.read
 #define StreamFlush Serial.flush
 #define StreamWrite Serial.write
 #define StreamPrint Serial.print
 #define StreamAvailable Serial.available
 #define FlowSerialBegin [](unsigned long baud) { Serial.begin(baud);}
 #endif