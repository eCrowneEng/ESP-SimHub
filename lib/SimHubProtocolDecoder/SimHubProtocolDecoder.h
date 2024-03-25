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
    #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n Disponible en buffer ");
        Serial.print(stream->available());
        Serial.print("\n");
        Serial.flush();
    #endif
    if(stream->available()){
       packetType=stream->read();
        #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n packetType ");
        Serial.print(packetType);
        Serial.print("\n");
        Serial.flush();
        #endif
    }
    if(packetType==0x09){
        packetType=stream->read();

         #if IC2_SERIAL_BYPASS_DEBUG
        Serial.print("\n packetType ");
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


        switch (packetType){
                case 0x01:
                   // callbacker->getEncoderPositionChangedCallback()(stream->read(),stream->read(),stream->read());
                    break;
                case 0x02:
                    //callbacker->getEncoderPositionChangedCallback()(stream->read(),stream->read(),stream->read());
                    break;
                case 0x03:
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
                    break;
                case 0x04:
                    // TODO: FIXME
                    //callbacker->getButtonCallback()((stream->read())*8+stream->read(),stream->read());
                    break;
                default:
                    break;
            }


        while (0 <stream->available()){
            Serial.write(stream->read());
         }
         stream->flush();
            
    }

}