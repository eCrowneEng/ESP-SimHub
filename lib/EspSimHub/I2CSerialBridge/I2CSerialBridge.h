
#include <FullLoopbackStream.h>
#include <Wire.h>
class I2CSerialBridge{
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
        #if DEBUG_TCP_BRIDGE
            Serial.printf("flushing with this much data: %d \n", availableLength);
        #endif
      // read the available data from the stream, and put in in the buffer
      char sbuf[availableLength];
      this->outgoingStream->readBytes(sbuf, availableLength);

        // send data to client
        WIRE.beginTransmission(address);
        size_t total = WIRE.write(sbuf, availableLength);
        WIRE.endTransmission();
        #if DEBUG_TCP_BRIDGE
            Serial.printf("\n ---> data sent to client %s: %d bytes \n", client->remoteIP().toString().c_str(), total);
            Serial.printf("%d %d\n",sbuf[0], sbuf[1]);
        #endif
      }
    }


     /** SETUP SERIAL BYPASS IC2 MASTER, USE WHEN THIS DEVICE COMMAND THE SENDING WORKFLOW*/
    void ic2SetupMaster(){
	WIRE.begin();

	while(!isSlaveAvailable()){
			Serial.printf("\n Slave device not available, retrying 1 sec later");
			delay(1000);
	};
    }

    private:
        /** TODO CONTROL THAT TRANSPORT LAYER IS READY AND CONNECTED*/
        void check_status(){

        }

     
    

uint8_t endWireTransmission(bool stop){
	uint8_t error=WIRE.endTransmission(stop);
	if(error=0){
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
    Serial.printf("\n Error code is %d",error);
	Serial.flush();
	return error;

}

bool isSlaveAvailable(){
     #if IC2_SERIAL_BYPASS_DEBUG
            Serial.print("\n Testing slave availability -> Beging transmission to  ");
            Serial.print(IC2_ADDRESS);
            Serial.println("\n");
     #endif
	WIRE.beginTransmission(IC2_ADDRESS);
    uint8_t error = endWireTransmission(true);

	if(error==0){
		Serial.printf("\n Slave device detected at address 8\n");
		return true;
	}
	return false;
}


};