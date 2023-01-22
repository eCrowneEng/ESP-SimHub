#ifndef __ARQSERIAL_H__
#define __ARQSERIAL_H__
//#define TESTFAIL

#include <Arduino.h>
#include "RingBuffer.h"

const uint8_t crc_table_crc8[256] PROGMEM = { 0,213,127,170,254,43,129,84,41,252,86,131,215,2,168,125,82,135,45,248,172,121,211,6,123,174,4,209,133,80,250,47,164,113,219,14,90,143,37,240,141,88,242,39,115,166,12,217,246,35,137,92,8,221,119,162,223,10,160,117,33,244,94,139,157,72,226,55,99,182,28,201,180,97,203,30,74,159,53,224,207,26,176,101,49,228,78,155,230,51,153,76,24,205,103,178,57,236,70,147,199,18,184,109,16,197,111,186,238,59,145,68,107,190,20,193,149,64,234,63,66,151,61,232,188,105,195,22,239,58,144,69,17,196,110,187,198,19,185,108,56,237,71,146,189,104,194,23,67,150,60,233,148,65,235,62,106,191,21,192,75,158,52,225,181,96,202,31,98,183,29,200,156,73,227,54,25,204,102,179,231,50,152,77,48,229,79,154,206,27,177,100,114,167,13,216,140,89,243,38,91,142,36,241,165,112,218,15,32,245,95,138,222,11,161,116,9,220,118,163,247,34,136,93,214,3,169,124,40,253,87,130,255,42,128,85,1,212,126,171,132,81,251,46,122,175,5,208,173,120,210,7,83,134,44,249 };
#define updateCrc(currentCrc, value) pgm_read_byte(&crc_table_crc8[currentCrc ^ value]);

typedef void(*IdleFunction) (bool);

class ARQSerial
{
private:

	byte partialdatabuffer[24];
	int Arq_LastValidPacket = 255;
	RingBuffer<uint8_t, 32> DataBuffer;
	IdleFunction idleFunction = 0;

#ifdef TESTFAIL
	int testfailidx = 0;
	int testfailidx2 = 0;
#endif

	int Arq_TimedRead()
	{
		int c;
		unsigned long fsr_startMillis = millis();
		do {
			if (idleFunction != 0) idleFunction(true);
			c = Serial.read();
			if (c >= 0) {
#ifdef TESTFAIL
				testfailidx = (testfailidx + 1) % 5000;
				if (testfailidx == 500)
					return random(255);

				if (testfailidx == 1000)
					return -1;
#endif
				return c;
			}
		} while (millis() - fsr_startMillis < 100);
		return -1;
	}

	void ProcessIncomingData() {
		int packetID, length, header, res, i, crc, nextpacketid;
		byte currentCrc;

		while (Serial.available() > 0) {
			header = Arq_TimedRead();
			//DebugPrintLn("hello1");
			currentCrc = 0;

			if (header == 0x01) {
				byte reason = 0x00;

				header = Arq_TimedRead();
				if (header != 0x01) {
					return;
				}

				if (reason == 0) {
					packetID = Arq_TimedRead();
					if (packetID < 0) {
						reason = 0x01;
					}
				}

				if (reason == 0) {
					length = Arq_TimedRead();
					if (length <= 0 || length > 32) {
						reason = 0x02;
					}
				}

				if (reason == 0)
				{
					for (i = 0; i < length && !reason; i++) {
						res = Arq_TimedRead();
						partialdatabuffer[i] = res;
						if (res < 0) reason = 0x05;
					}
				}

				if (reason == 0) {
					crc = Arq_TimedRead();
					if (crc < 0) {
						reason = 0x03;
					}
				}

				if (reason == 0) {
					currentCrc = updateCrc(currentCrc, packetID);
					currentCrc = updateCrc(currentCrc, length);
					for (i = 0; i < length; i++) {
						currentCrc = updateCrc(currentCrc, partialdatabuffer[i]);
					}

					if (crc != currentCrc) {
						reason = 0x04;
					}
				}

				if (reason == 0) {
					nextpacketid = Arq_LastValidPacket > 127 ? 0 : Arq_LastValidPacket + 1;

					if (packetID == nextpacketid || packetID == 255) {
						for (i = 0; i < length; i++) {
							DataBuffer.push(partialdatabuffer[i]);
						}
						Arq_LastValidPacket = packetID;
					}
#ifdef TESTFAIL
					testfailidx = (testfailidx + 1) % 5000;
					if (testfailidx != 788) {
						SendAcq(packetID);
					}
#else
					SendAcq(packetID);
#endif
				}

				if (reason > 0) {
					SendNAcq(Arq_LastValidPacket, reason);
				}
			}
		}
	}

	void SendAcq(uint8_t packetId)
	{
		Serial.write(0x03);
		Serial.write(packetId);
		Serial.flush();
	}

	void SendNAcq(uint8_t lastKnownValidPacket, byte reason)
	{
		Serial.write(0x04);
		Serial.write(lastKnownValidPacket);
		Serial.write(reason);
		Serial.flush();
	}

public:

	void setIdleFunction(IdleFunction function) {
		idleFunction = function;
	}

	void CustomPacketStart(byte packetType, uint8_t length) {
		Serial.write(0x09);
		Serial.write(packetType);
		Serial.write(length);
	}

	void CustomPacketSendByte(byte data) {
		Serial.write(data);
	}

	void CustomPacketEnd() {
		//Serial.write(0x00);
	}

	int read() {
		unsigned long fsr_startMillis = millis();
		do {
			if (idleFunction != 0) idleFunction(false);

			if (DataBuffer.size() > 0) {
				uint8_t res = 0;
				DataBuffer.pop(res);
				return (int)res;
			}

			ProcessIncomingData();
		} while (millis() - fsr_startMillis < 400 || DataBuffer.size() > 0);

		//DebugPrintLn("Read timeout !");
		return -1;
	}

	int Available() {
		if (idleFunction != 0) idleFunction(false);
		if (DataBuffer.size() == 0) {
			ProcessIncomingData();
		}
		return DataBuffer.size();
	}

	void Write(byte data) {
		Serial.write(0x08);
		Serial.write(data);
		Serial.flush();
	}

	void Print(char data)
	{
		Write((byte)data);
	}

	void Print(const char str[]) {
		int len = strlen(str);
		Serial.write(0x06);
		Serial.write(len);
		Serial.write(str);
		Serial.write(0x20);
		Serial.flush();
	}

	void WriteString(String& data)
	{
		int len = data.length();
		Serial.write(0x06);
		Serial.write(len);
		Serial.print(data);
		Serial.write(0x20);
		Serial.flush();
	}

	void PrintString(const char str[]) {
		int len = strlen(str);
		Serial.write(0x06);
		Serial.write(len);
		Serial.write(str);
		Serial.write(0x20);
		Serial.flush();
	}

	void PrintLn(const char str[]) {
		int len = strlen(str);
		Serial.write(0x06);
		Serial.write(len + 1);
		Serial.write(str);
		Serial.write('\n');
		Serial.write(0x20);
		Serial.flush();
	}

	void PrintLn(String& data)
	{
		Serial.write(0x06);
		Serial.write(data.length() + 1);
		Serial.print(data);
		Serial.print('\n');
		Serial.write(0x20);
		Serial.flush();
	}

	void PrintLn() {
		Write('\n');
	}

	String ReadStringUntil(char terminator1, char terminator2) {
		String ret;
		int c = read();
		while (c >= 0 && c != terminator1 && c != terminator2)
		{
			ret += (char)c;
			c = read();
		}
		return ret;
	}

	void ReadStringUntil(char buffer[], char terminator) {
		int pos = 0;

		int c = read();
		while (c >= 0 && c != terminator)
		{
			buffer[pos] = (char)c;
			c = read();
			pos++;
		}
		buffer[pos] = 0;
		
	}

	String ReadStringUntil(char terminator1) {
		String ret;
		int c = read();
		while (c >= 0 && c != terminator1)
		{
			ret += (char)c;
			c = read();
		}
		return ret;
	}

	void DebugPrintLn(String& data)
	{
		Serial.write(0x07);
		Serial.write(data.length() + 1);
		Serial.print(data);
		Serial.print('\n');
		Serial.write(0x20);
		Serial.flush();
	}

	void DebugPrint(char data)
	{
		Serial.write(0x07);
		Serial.write(1);
		Serial.print(data);
		Serial.write(0x20);
		Serial.flush();
	}

	void DebugPrintLn(const char str[]) {
		Serial.write(0x07);
		Serial.write((byte)(strlen(str) + 1));
		Serial.print(str);
		Serial.print('\n');
		Serial.write(0x20);
		Serial.flush();
	}
};

#endif