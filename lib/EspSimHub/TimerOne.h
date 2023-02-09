#include <Arduino.h>
#include <map>
#include <stdexcept>

double frequency;
#ifdef ESP32
int currentChannel = 0;
std::map<char, int> pinToChannel{};
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
        frequency = (double)1 / microseconds * 1000000;
#ifdef ESP8266
        analogWriteFreq(freq);
        // set duty cycle range from 0 to 1023, per requirement of simhub params (10 bit)
        analogWriteRange(1023);
#endif
    }

    void pwm(char pin, unsigned int duty) __attribute__((always_inline))
    {
#ifdef ESP32
        // While I don't love all of this, it may be ok because it's only for the esp32
        //  in theory the map access here has logarithmic performance based on the size
        //  which means it's not O(1) as if we didn't have to map pins to channels. Also,
        //  there will be at most 16 pwm channels
        // Alternatively, we could create allocate an array of the size of the maximum pin
        //  number, and store the channel in that array, so that we can request the position
        //  in the array of the pin being allocated to a channel, that way access is faster
        //  but we use more memory
        // Ultimately, this will depend on actual performance

        int existingChannel;
        try
        {
            existingChannel = pinToChannel.at(pin);
        }
        catch (const std::out_of_range &oor)
        {
            int newChannel = currentChannel++;
            if (newChannel > 15)
            {
                throw std::out_of_range("too many pwm channels");
            }
            ledcSetup(newChannel, frequency, 10);
            existingChannel = newChannel;
        }
        // 10 bits resolution
        ledcWrite(existingChannel, duty);
#else
        // write duty cycle
        analogWrite(pin, duty);
#endif
    }
};

extern FakeTimerOneLibrary Timer1;