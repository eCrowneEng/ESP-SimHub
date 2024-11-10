#include <Arduino.h>


typedef void(*SHButtonChangedEventCallBack) (int, byte);                        //function pointer prototype
typedef void(*SHRotaryEncoderPositionChangedEventCallBack) (int, int, byte);  //function pointer prototype
/***
 * | PACKET HEADER  | SIZE | DSCRIPTION                     | 
 * | 0x01           |  3   | ENCODER  SIZE 3
 * | 0X02           |  2   | ENCODER  SIZE 2
 * | 0X03           |  2   | BUTTON STATE  SIZE 2
 * | 0X04           |  3   | TM1638 BUTTONS STATE SIZE 3
 * 
*/
byte packetsHeaders[]={0x01,0x02,0x03,0x04};
byte packetsPayloadLength[]={3,2,2,3};

class EventCallBackManager{
     SHButtonChangedEventCallBack shButtonChangedCallback;
     SHRotaryEncoderPositionChangedEventCallBack SHRotaryEncoderPositionChangedCallback;

    public:
        void setButtonCallBack(SHButtonChangedEventCallBack callback){
            shButtonChangedCallback=callback;
        }
         SHButtonChangedEventCallBack getButtonCallback(){
           return shButtonChangedCallback;
        }

         void setEncoderPositionChangedCallback(SHRotaryEncoderPositionChangedEventCallBack callback){
            SHRotaryEncoderPositionChangedCallback=callback;
        }
         SHRotaryEncoderPositionChangedEventCallBack getEncoderPositionChangedCallback(){
           return SHRotaryEncoderPositionChangedCallback;
        }
};


static void decodeBuffer(EventCallBackManager *callbacker,Stream  *stream){
    byte packetType=0x0;
    int size=-1;
   //TODO: REMOVE IS FOR TESTING
    // while(stream->available()){
    //         Serial.write(stream->read());
    //     }
    // return;
    #define IC2_SERIAL_BYPASS_DEBUG false
    #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n Disponible en buffer ");
        Serial.print(stream->available());
        Serial.print("\n");
        Serial.flush();
    #endif
    if(stream->available()){
       packetType=stream->read();
        #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n Initial packetType  ");
        Serial.print(packetType);
        Serial.print("\n");
        Serial.flush();
        #endif
    }

    // CUSTOM PACKETS
    if(packetType==0x09){
        packetType=stream->read();
         #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n Accepted packetType ");
        Serial.print(packetType);
        Serial.print("\n");
        Serial.flush();
        #endif

        size=stream->read();

        #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n Payload Size is ");
        Serial.print(size);
        Serial.print("\n");
        Serial.flush();
        #endif

        if(packetType==0x01){
             int encoderId=stream->read();
             byte direction=stream->read();
             int position=stream->read(); 
             callbacker->getEncoderPositionChangedCallback()(encoderId,position,direction);
        }

        if(packetType==0x02){
            int encoderId=stream->read();
            byte direction=stream->read();
            callbacker->getEncoderPositionChangedCallback()(encoderId,direction,0xD7); // 0XD7 Identify a button status changed 
        }

        if(packetType==0x03){
            int buttonId;
            buttonId=stream->read();
            byte status;
            status=stream->read();
            #if IC2_SERIAL_BYPASS_DEBUG
                Serial.print("\n BUtton state changed ");
                Serial.println(buttonId);
                Serial.println(status);
                Serial.flush();
            #endif
            callbacker->getButtonCallback()(buttonId,status);
        }

        while (0 <stream->available()){
            Serial.write(stream->read());
         }
         stream->flush();
            
    }

}