#ifndef __SHMAX72217Segment_H__
#define __SHMAX72217Segment_H__
#include <Arduino.h>
#include <LedController.hpp>

class SHMAX72217Segment {

private:
	byte MAX7221_ByteReorder(byte x)
	{
		x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
		x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
		x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
		return (x >> 1) | ((x & 1) << 7);
	}
	int luminosity[6]{-1,-1,-1,-1,-1,-1};
	LedController MAX7221;
public:


	void begin(int numSegments, int dataPin, int clkPin, int csPin) {
		MAX7221.init(dataPin, clkPin, csPin, numSegments);
		for(unsigned int i = 0; i < MAX7221.getSegmentCount(); i++) {
			for(unsigned int j = 0; j < 8; j++) {
				MAX7221.setRow(i, j, 0x00);
			}
		}
		MAX7221.setIntensity(15);
	}

	void read()
	{
		pinMode(LED_BUILTIN, OUTPUT);
		digitalWrite(LED_BUILTIN, HIGH);

		for (int j = 0; j < MAX7221.getSegmentCount(); j++) {
			// Wait for display data
			int newIntensity = FlowSerialTimedRead();
			if (newIntensity != luminosity[j]) {
				// called multiple times even if unnecessary due to the data being sent over the wire
				MAX7221.setIntensity(newIntensity * 2 + 1);
				luminosity[j] = newIntensity;
			}
			for (int i = 0; i < 8; i++) {
				MAX7221.setRow(j, 7 - i, MAX7221_ByteReorder((char)FlowSerialTimedRead()));
			}
		}
	}

};
#endif