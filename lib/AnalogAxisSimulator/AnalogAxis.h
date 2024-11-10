#include <Arduino.h>
typedef void(*SHAxisChanged) (int, int);

class AnalogAxisSimulator{
    int axisIdx=1;
    int lastAxisValue = -1;
    int minimumInputValue=0;
	int maximumInputValue=1024;
	int samplingRate=10;
    float  exponentialFactor=1;
    unsigned long lastGeneratedReading=millis();
    SHAxisChanged shAxisChangedCallback=nullptr;
    public:

    AnalogAxisSimulator(int axisIdx){
        this->axisIdx=axisIdx;
    }
    bool read(int samplingRate);

    void setCallBack(SHAxisChanged callback){
		shAxisChangedCallback=callback;
	}

    float analogReadXXbit(byte bits_of_precision);
};