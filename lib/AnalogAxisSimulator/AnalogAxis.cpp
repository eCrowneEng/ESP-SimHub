#include "AnalogAxis.h"

int pot = -1;
bool AnalogAxisSimulator::read(int samplingRate){

    
        if(millis()-lastGeneratedReading>300){
            pot=analogReadXXbit(samplingRate);
            lastGeneratedReading=millis();
        }

         if (lastAxisValue != pot) { 
            Serial.println("\nAnalogAxisSimulator::setAnalogReading");
            Serial.println(pot);
		 	lastAxisValue = pot;

		 	int mapped = map(pot, minimumInputValue, maximumInputValue, 0, 1024);
            Serial.print("\nMapped:");
            Serial.print(mapped);
		 	float mapped2 = min((float)1, max((float)0,(float) ((float)mapped / (float)1024)));
            Serial.print("\nMapped2:");
            Serial.print(mapped2);

		 	if (exponentialFactor != 1) {
		 		mapped2 = pow(mapped2, 1.0 / (float)exponentialFactor);
		 	}

		 	mapped2 = mapped2 * 1024;
            Serial.print("\nMapped2 Powed:");
            Serial.print(mapped2);
		 	if(this->shAxisChangedCallback!=nullptr){
                Serial.println("\nCallback defined");
		 		shAxisChangedCallback(axisIdx,mapped2);
		 	}
		 }
    return false;
		
}

	float AnalogAxisSimulator::analogReadXXbit(byte bits_of_precision)
	{
		uint8_t n = bits_of_precision - 10;
		unsigned long oversample_num = 1 << (2 * n);
		uint8_t divisor = 1 << n;
		unsigned long reading_sum = 0;
		unsigned long inner_sum = 0;
		for (unsigned long j = 0; j < oversample_num; j++)
		{
			inner_sum +=random(-1024,1024);
		}
		unsigned int reading = (inner_sum + (unsigned long)divisor / 2UL) >> n;
		reading_sum += reading;
		return  (float)reading_sum;
	}