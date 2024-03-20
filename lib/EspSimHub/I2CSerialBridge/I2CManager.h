#pragma once
#include "IC2TransportMaster.h"
#include "I2CSeriaBypass.h"

/***
 * "This Manager is responsible for configuring the behavior based on the master-slave role for I2C communication."
*/
class IC2TransportManager  {
    public:
       static IC2TransportMaster tm;
       static IC2TransportSlave ts;

    static void setup(FullLoopbackStream *outgoingStream){
        #if IC2_MASTER
            #if IC2_SERIAL_BYPASS_DEBUG
                Serial.begin(115200);
                Serial.print("\nSetup as Master\n");
                Serial.flush();
            #endif
        tm.setup(outgoingStream);
        #endif

        #if IC2_SLAVE
        Serial.print("Setup as Slave");
        Serial.flush();
        ts.setup(outgoingStream);
        #endif
    }
    static void loop(){
        #ifdef IC2_MASTER
        tm.loop();
        #endif

        #ifdef IC2_SLAVE
        ts.loop();
        #endif
    }

    static void flush(){
        #ifdef IC2_MASTER
            tm.flush();
        #endif

        #ifdef IC2_SLAVE
            ts.flush();
        #endif
    }
};

#if IC2_MASTER
#define StreamAvailable WIRE.available
#define FlowSerialFlush WIRE.flush
#define StreamFlush WIRE.flush
#define StreamWrite outgoingStream.write
#define StreamPrint WIRE.print
/** SETUP SERIAL BYPASS IC2 SLAVE, USE WHEN THIS DEVICE IS CONNECTED TO SIMHUB*/
#define FlowSerialBegin [](unsigned long baud) { Serial.printf("Hola mundo");}
#endif
#if IC_SLAVE
#define StreamRead Serial.read
#define StreamFlush Serial.flush
#define StreamWrite Serial.write
#define StreamPrint Serial.print
#define StreamAvailable Serial.available
#define FlowSerialBegin [](unsigned long baud) { Serial.begin(baud);}
#endif