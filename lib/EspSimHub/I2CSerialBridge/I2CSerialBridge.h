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
        Wire.beginTransmission(address);
        size_t total = Wire.write(sbuf, availableLength);
        Wire.endTransmission();
#if DEBUG_TCP_BRIDGE
	  Serial.printf("\n ---> data sent to client %s: %d bytes \n", client->remoteIP().toString().c_str(), total);
    Serial.printf("%d %d\n",sbuf[0], sbuf[1]);
#endif
      }
    }
  

    private:
        /** TODO CONTROL THAT TRANSPORT LAYER IS READY AND CONNECTED*/
        void check_status(){

        }

};