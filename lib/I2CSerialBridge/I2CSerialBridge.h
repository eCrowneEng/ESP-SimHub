
#include <FullLoopbackStream.h>
#include <Wire.h>

#ifndef WIRE
#error WIRE must be settled to have a correct custom wire config in your MASTER config.
#endif

class    I2CSerialBridge{
      FullLoopbackStream *outgoingStream;
      uint8_t address;
    public:

        I2CSerialBridge(uint8_t address) {
            this->address=address;
            // CONFIG AS MASTER OR SLAVE WITH ADDRESS
        }

        void setup(FullLoopbackStream *outgoingStream){
            this->outgoingStream = outgoingStream;
        }

        void loop() {
        #if I2C_SERIAL_BYPASS_DEBUG
          //  Serial.print("\n I2CSerialBridge - loop");
          //  Serial.flush();
        #endif
        // put your main code here, to run repeatedly
        check_status();
        this->flush();
    }

    void flush() {
    // if there is data available in the stream, it's meant
    //  to go from Serial port to the slave
    size_t availableLength = this->outgoingStream->available();
    if (availableLength)
    {
        #if I2C_SERIAL_BYPASS_DEBUG
            Serial.println("\n I2CSerialBridge - flush");
            Serial.print("\nflushing with this much data:");
            Serial.print(availableLength);
        #endif

        
      // read the available data from the stream, and put in in the buffer
      char sbuf[availableLength];
      this->outgoingStream->readBytes(sbuf, availableLength);

        // send data to client
        WIRE.beginTransmission(address);
        size_t total = WIRE.write(sbuf, availableLength);
        WIRE.endTransmission();
        //endWireTransmission(false);
        // DEBUG OUTPUT STREAM
        for (int i=0;i<availableLength;i++){
            Serial.write(sbuf[i]);
        }
            Serial.flush();
        /// END DEBUG OUTPUT STREAM

        #if I2C_SERIAL_BYPASS_DEBUG
 //           Serial.println("\n I2CSerialBridge - flush");
//            Serial.printf("\n ---> data sent to client %s: %d bytes \n", I2C_ADDRESS, total);
            // Serial.printf("%d %d\n",sbuf[0], sbuf[1]);
        #endif
      }
    }


     /** SETUP SERIAL BYPASS I2C MASTER, USE WHEN THIS DEVICE COMMAND THE SENDING WORKFLOW*/
    void i2cSetupMaster(){
        #if I2C_SERIAL_BYPASS_DEBUG
            Serial.print("WIRE.BEGIN(), initializing....");
        #endif
        WIRE.begin();
          #if I2C_SERIAL_BYPASS_DEBUG
            Serial.print("WIRE CONFIGURING TIMEOUT 300 ms .... ");
        #endif
        WIRE.setTimeout(300);

        while(!isSlaveAvailable()){
                #if I2C_SERIAL_BYPASS_DEBUG
                    Serial.print("\n Slave device not available, retrying 1 sec later");
                #endif
                delay(1000);
        };
    }

    private:
        /** TODO CONTROL THAT TRANSPORT LAYER IS READY AND CONNECTED*/
        void check_status(){
        #if I2C_SERIAL_BYPASS_DEBUG
          //  Serial.println("\n I2CSerialBridge - check_status");
          //  Serial.flush();
        #endif
        }

     
    

uint8_t endWireTransmission(bool stop){
	uint8_t error=WIRE.endTransmission(stop);
	if(error==0){
		Serial.print("\n Correct wire close \n");
	}
	if(error==1){
		Serial.print("\n Wire buffer exausted \n");
	}
	if(error==2){
		Serial.print("\n received NACK on transmit of address.\n");
	}
	if(error==3){
		Serial.print("received NACK on transmit of data.");
	}
	if(error==4){
		Serial.print(" other error.");
	}
	if(error==5){
		Serial.print("timeout");
	}
    Serial.print("\n Error code is ");
    Serial.print(error);
    Serial.println();
	Serial.flush();
	return error;

}

bool isSlaveAvailable(){
     #if I2C_SERIAL_BYPASS_DEBUG
            Serial.print("\n Testing slave availability -> Beging transmission to  ");
            Serial.print(I2C_ADDRESS);
            Serial.println("\n");
     #endif
	WIRE.beginTransmission(I2C_ADDRESS);
    uint8_t error = endWireTransmission(true);

	if(error==0){
		Serial.print("\n Slave device detected at address ");
        Serial.print(I2C_ADDRESS);
        Serial.print("\n");
		return true;
	}
	return false;
}


};