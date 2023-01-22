#ifndef __SHMAX72217Segment_H__
#define __SHMAX72217Segment_H__
#include <Arduino.h>
#include "SHLedControl.h"

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
	SHLedControl MAX7221;
public:


	void begin(int screens, int DataPin, int ClkPin, int LoadPin) {
		MAX7221.begin(DataPin, ClkPin, LoadPin, screens);
		for (int i = 0; i < screens; i++) {
			MAX7221.shutdown(i, false);
			MAX7221.setIntensity(i, 15);
			MAX7221.clearDisplay(i);
		}
	}

	void read()
	{
		for (int j = 0; j < MAX7221.getDeviceCount(); j++) {
			// Wait for display data
			int newIntensity = FlowSerialTimedRead();
			if (newIntensity != luminosity[j]) {
				MAX7221.setIntensity(j, newIntensity * 2 + 1);
				luminosity[j] = newIntensity;
			}
			for (int i = 0; i < 8; i++) {
				MAX7221.setRow(j, 7 - i, MAX7221_ByteReorder((char)FlowSerialTimedRead()));
			}
		}

	}

};
#endif