
#ifndef SHLedControl_h
#define SHLedControl_h

 //the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

#include <avr/pgmspace.h>

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class SHLedControl {
private:
	/* The array for shifting the data to the devices */
	byte spidata[16];
	/* Send out a single command to the device */

	/* We keep track of the led-status for all 8 devices in this array */
	byte status[64];
	/* Data is shifted out of this pin*/
	int SPI_MOSI;
	/* The clock is signaled on this pin */
	int SPI_CLK;
	/* This one is driven LOW for chip selectzion */
	int SPI_CS;
	/* The maximum number of devices we use */
	int maxDevices;
	void spiTransfer(int addr, volatile byte opcode, volatile byte data) {
		//Create an array with the data to shift out
		int offset = addr * 2;
		int maxbytes = maxDevices * 2;

		for (int i = 0; i < maxbytes; i++)
			spidata[i] = (byte)0;
		//put our device data into the array
		spidata[offset + 1] = opcode;
		spidata[offset] = data;
		//enable the line
		o_digitalWrite(SPI_CS, LOW);
		//Now shift out the data
		for (int i = maxbytes; i > 0; i--)
			o_shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[i - 1]);
		//latch the data onto the display
		o_digitalWrite(SPI_CS, HIGH);
	}
public:
	/*
	 * Create a new controler
	 * Params :
	 * dataPin		pin on the Arduino where data gets shifted out
	 * clockPin		pin for the clock
	 * csPin		pin for selecting the device
	 * numDevices	maximum number of devices that can be controled
	 */
	void begin(int dataPin, int clkPin, int csPin, int numDevices) {
		SPI_MOSI = dataPin;
		SPI_CLK = clkPin;
		SPI_CS = csPin;
		if (numDevices <= 0 || numDevices > 8)
			numDevices = 8;
		maxDevices = numDevices;
		pinMode(SPI_MOSI, OUTPUT);
		pinMode(SPI_CLK, OUTPUT);
		pinMode(SPI_CS, OUTPUT);
		o_digitalWrite(SPI_CS, HIGH);
		SPI_MOSI = dataPin;
		for (int i = 0; i < 64; i++)
			status[i] = 0x00;
		for (int i = 0; i < maxDevices; i++) {
			spiTransfer(i, OP_DISPLAYTEST, 0);
			//scanlimit is set to max on startup
			setScanLimit(i, 7);
			//decode is done in source
			spiTransfer(i, OP_DECODEMODE, 0);
			clearDisplay(i);
			//we go into shutdown-mode on startup
			shutdown(i, true);
		}
	}

	/*
	 * Gets the number of devices attached to this LedControl.
	 * Returns :
	 * int	the number of devices on this LedControl
	 */
	int getDeviceCount() {
		return maxDevices;
	}

	/*
	 * Set the number of digits (or rows) to be displayed.
	 * See datasheet for sideeffects of the scanlimit on the brightness
	 * of the display.
	 * Params :
	 * addr	address of the display to control
	 * limit	number of digits to be displayed (1..8)
	 */
	void setScanLimit(int addr, int limit) {
		if (addr < 0 || addr >= maxDevices)
			return;
		if (limit >= 0 && limit < 8)
			spiTransfer(addr, OP_SCANLIMIT, limit);
	}
	/*
	 * Set the brightness of the display.
	 * Params:
	 * addr		the address of the display to control
	 * intensity	the brightness of the display. (0..15)
	 */
	void setIntensity(int addr, int intensity) {
		if (addr < 0 || addr >= maxDevices)
			return;
		if (intensity >= 0 && intensity < 16)
			spiTransfer(addr, OP_INTENSITY, intensity);
	}
	/*
	 * Switch all Leds on the display off.
	 * Params:
	 * addr	address of the display to control
	 */
	void clearDisplay(int addr) {
		int offset;

		if (addr < 0 || addr >= maxDevices)
			return;
		offset = addr * 8;
		for (int i = 0; i < 8; i++) {
			status[offset + i] = 0;
			spiTransfer(addr, i + 1, status[offset + i]);
		}
	}
	/*
	 * Set the status of a single Led.
	 * Params :
	 * addr	address of the display
	 * row	the row of the Led (0..7)
	 * col	the column of the Led (0..7)
	 * state	If true the led is switched on,
	 *		if false it is switched off
	 */
	void setLed(int addr, int row, int column, boolean state) {
		int offset;
		byte val = 0x00;

		if (addr < 0 || addr >= maxDevices)
			return;
		if (row < 0 || row>7 || column < 0 || column>7)
			return;
		offset = addr * 8;
		val = B10000000 >> column;
		if (state)
			status[offset + row] = status[offset + row] | val;
		else {
			val = ~val;
			status[offset + row] = status[offset + row] & val;
		}
		spiTransfer(addr, row + 1, status[offset + row]);
	}

	/*
	 * Set all 8 Led's in a row to a new state
	 * Params:
	 * addr	address of the display
	 * row	row which is to be set (0..7)
	 * value	each bit set to 1 will light up the
	 *		corresponding Led.
	 */
	void setRow(int addr, int row, byte value) {
		int offset;
		if (addr < 0 || addr >= maxDevices)
			return;
		if (row < 0 || row>7)
			return;
		offset = addr * 8;
		status[offset + row] = value;
		spiTransfer(addr, row + 1, status[offset + row]);
	}

	/*
	 * Set all 8 Led's in a column to a new state
	 * Params:
	 * addr	address of the display
	 * col	column which is to be set (0..7)
	 * value	each bit set to 1 will light up the
	 *		corresponding Led.
	 */
	void setColumn(int addr, int col, byte value) {
		byte val;

		if (addr < 0 || addr >= maxDevices)
			return;
		if (col < 0 || col>7)
			return;
		for (int row = 0; row < 8; row++) {
			val = value >> (7 - row);
			val = val & 0x01;
			setLed(addr, row, col, val);
		}
	}

	void o_digitalWrite(uint8_t pin, uint8_t val) {
		uint8_t bit = digitalPinToBitMask(pin);
		uint8_t port = digitalPinToPort(pin);
		volatile uint8_t *out;

		out = portOutputRegister(port);

		if (val == LOW) {
			*out &= ~bit;
		}
		else {
			*out |= bit;
		}
	}

	void shutdown(int addr, bool b) {
		if (addr < 0 || addr >= maxDevices)
			return;
		if (b)
			spiTransfer(addr, OP_SHUTDOWN, 0);
		else
			spiTransfer(addr, OP_SHUTDOWN, 1);
	}

	void o_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
	{
		uint8_t i;

		for (i = 0; i < 8; i++) {
			if (bitOrder == LSBFIRST)
				o_digitalWrite(dataPin, !!(val & (1 << i)));
			else
				o_digitalWrite(dataPin, !!(val & (1 << (7 - i))));

			o_digitalWrite(clockPin, HIGH);
			o_digitalWrite(clockPin, LOW);
		}
	}
};

#endif	//LedControl.h
