#include <Arduino.h>

// Fake TimerOne used for Arduinos that may not be necessary for ESP
//  This is a compatibility layer between what SimHub uses for Arduino
//  and what we need for the ESP
class FakeTimerOneLibrary
{
public:
    void initialize(unsigned long microseconds = 1000000) __attribute__((always_inline))
    {
        // convert amplitude in microseconds to frequency
        double freq = (double)1 / microseconds * 1000000;
        analogWriteFreq(freq);
        // set duty cycle range from 0 to 1024, per requirement of simhub params
        analogWriteRange(1023);
    }

    void pwm(char pin, unsigned int duty) __attribute__((always_inline)) 
    {
        // write duty cycle
        analogWrite(pin, duty);
    }
};

extern FakeTimerOneLibrary Timer1;