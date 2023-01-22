#ifndef __SHROTARYENCODER_H__
#define __SHROTARYENCODER_H__

#include <Arduino.h>
#include "SHFastIO.h"
#include "SHDebouncer.h"

#define R_START	   0x0
#define DIR_CW    0x10
#define DIR_CCW   0x20

// Full steps
#define FS_R_CW_FINAL   0x1
#define FS_R_CW_BEGIN   0x2
#define FS_R_CW_NEXT    0x3
#define FS_R_CCW_BEGIN  0x4
#define FS_R_CCW_FINAL  0x5
#define FS_R_CCW_NEXT   0x6

// Half steps
#define HS_R_CCW_BEGIN   0x1
#define HS_R_CW_BEGIN    0x2
#define HS_R_START_M     0x3
#define HS_R_CW_BEGIN_M  0x4
#define HS_R_CCW_BEGIN_M 0x5

static const unsigned char fullStepsTable[7][4] =
{
	// 00         01           10           11
	{R_START,		FS_R_CW_BEGIN,	FS_R_CCW_BEGIN,	R_START},           // R_START
	{FS_R_CW_NEXT,  R_START,		FS_R_CW_FINAL,	R_START | DIR_CW},  // R_CW_FINAL
	{FS_R_CW_NEXT,  FS_R_CW_BEGIN,  R_START,		R_START},           // R_CW_BEGIN
	{FS_R_CW_NEXT,  FS_R_CW_BEGIN,  FS_R_CW_FINAL,  R_START},           // R_CW_NEXT
	{FS_R_CCW_NEXT, R_START,		FS_R_CCW_BEGIN,	R_START},           // R_CCW_BEGIN
	{FS_R_CCW_NEXT, FS_R_CCW_FINAL, R_START,		R_START | DIR_CCW}, // R_CCW_FINAL
	{FS_R_CCW_NEXT, FS_R_CCW_FINAL, FS_R_CCW_BEGIN, R_START}            // R_CCW_NEXT
};

static const unsigned char halfStepsTable[][4] =
{
	// 00                  01              10						11
	{HS_R_START_M,           HS_R_CW_BEGIN,     HS_R_CCW_BEGIN,		R_START},           // R_START (00)
	{HS_R_START_M | DIR_CCW, R_START,			HS_R_CCW_BEGIN,		R_START},           // R_CCW_BEGIN
	{HS_R_START_M | DIR_CW,  HS_R_CW_BEGIN,     R_START,			R_START},           // R_CW_BEGIN
	{HS_R_START_M,           HS_R_CCW_BEGIN_M,  HS_R_CW_BEGIN_M,	R_START},           // R_START_M (11)
	{HS_R_START_M,           HS_R_START_M,      HS_R_CW_BEGIN_M,	R_START | DIR_CW},  // R_CW_BEGIN_M 
	{HS_R_START_M,           HS_R_CCW_BEGIN_M,  HS_R_START_M,		R_START | DIR_CCW}  // R_CCW_BEGIN_M
};

typedef void(*SHRotaryEncoderPositionChanged) (int, int, byte);

class SHRotaryEncoder {
private:

	FastDigitalPin outputA; // CLK
	FastDigitalPin outputB; // DT
	FastDigitalPin button;

	int counter = 0;
	bool halfSteps = false;

	uint8_t inputLastState;
	uint8_t buttonState;

	int buttonLastState;
	unsigned long lastbuttonchange;
	unsigned long positionLastChanged;
	unsigned long directionLastChange = 255;
	SHDebouncer buttonDebouncer;
	uint8_t direction;

	byte id;
	SHRotaryEncoderPositionChanged positionChangedCallback;

public:

	void begin(uint8_t outputAPin, uint8_t outputBPin, int buttonPin, bool reverse, bool enablePullup, byte encoderid, bool half, SHRotaryEncoderPositionChanged changedcallback) {
		halfSteps = half;
		buttonDebouncer.begin(50);
		outputA.begin((!reverse) ? outputAPin : outputBPin);
		outputB.begin((!reverse) ? outputBPin : outputAPin);

		pinMode(outputAPin, enablePullup ? INPUT_PULLUP : INPUT);
		pinMode(outputBPin, enablePullup ? INPUT_PULLUP : INPUT);

		button.begin(buttonPin);
		if (buttonPin > 0) {
			pinMode(buttonPin, enablePullup ? INPUT_PULLUP : INPUT);
		}

		id = encoderid;
		inputLastState = 0;
		buttonLastState = button.digitalRead();
		positionChangedCallback = changedcallback;
	}

	uint8_t getDirection(uint8_t delay, unsigned long referenceTime) {
		if (directionLastChange != 255 && (referenceTime - positionLastChanged) < delay) {
			return directionLastChange;
		}
		return 255;
	}

	uint8_t getPressed() {
		return button.isValid() && !buttonLastState;
	}

	void read() {
		if (!halfSteps)
			inputLastState = fullStepsTable[inputLastState & 0xf][(outputB.digitalRead() << 1) | outputA.digitalRead()];
		else {
			inputLastState = halfStepsTable[inputLastState & 0xf][(outputB.digitalRead() << 1) | outputA.digitalRead()];
		}

		direction = (inputLastState & 0x30);

		if (direction == DIR_CCW) {
			counter++;
			positionChangedCallback(id, counter, 0);
			positionLastChanged = millis();
			directionLastChange = 0;
		}
		else if (direction == DIR_CW) {
			counter--;
			positionChangedCallback(id, counter, 1);
			positionLastChanged = millis();
			directionLastChange = 1;
		}

		if (button.isValid()) {
			buttonState = button.digitalRead();
			if (buttonState != buttonLastState) {
				if (buttonDebouncer.Debounce()) {
					positionChangedCallback(id, counter, buttonState == HIGH ? 2 : 3);
					buttonLastState = buttonState;
				}
			}
		}
	}
};

#endif