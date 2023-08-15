#include <Arduino.h>
#include <EspSimHub.h>
#include <stdexcept>

double frequency;
#ifdef ESP32
int currentChannel = 0;
// 44 is the maximum pin number we support for pwm, if you need higher, just allocate more and add more initializers
//  it has to be initialized as -1, because 0 is a valid channel
int pinToChannel[44] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
#endif

// Fake TimerOne used for Arduinos that may not be necessary for ESP
//  This is a compatibility layer between what SimHub uses for Arduino
//  and what we need for the ESP
class FakeTimerOneLibrary
{
public:
    void initialize(unsigned long microseconds = 1000000) __attribute__((always_inline))
    {
        // convert amplitude in microseconds to frequency
        //frequency = (double)1 / microseconds * 1000000;
		
#ifdef ESP32
   frequency = 5000;
#endif
    }

    void pwm(char pin, unsigned int duty) __attribute__((always_inline))
    {
#ifdef ESP32
        int existingChannel;
        existingChannel = pinToChannel[(int)pin];
        if (existingChannel < 0)
        {
            int newChannel = currentChannel++;
            pinToChannel[(int)pin] = newChannel;
            if (newChannel > 15)
            {
                throw std::out_of_range("too many pwm channels");
            }
            ledcSetup(newChannel, frequency, 10);
            existingChannel = newChannel;
        }
        // 10 bits resolution
        ledcWrite(existingChannel, duty);
#endif
#ifdef ESP8266
        // write duty cycle
        analogWrite(pin, duty);
#endif
    }
};

extern FakeTimerOneLibrary Timer1;