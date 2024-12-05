#ifndef FlowSerialBegin
#define FlowSerialBegin Serial.begin
#define FlowSerialFlush Serial.flush
#endif

#include "ArqSerial.h"
ARQSerial arqserial;

#define FlowSerialAvailable() arqserial.Available()
#define FlowSerialTimedRead() arqserial.read()
#define FlowSerialWrite(data) arqserial.Write(data)

String FlowSerialReadStringUntil(char terminator) { return arqserial.ReadStringUntil(terminator); }
String FlowSerialReadStringUntil(char terminator1, char terminator2) { return arqserial.ReadStringUntil(terminator1, terminator2); }
void FlowSerialReadStringUntil(char buffer[], char terminator){ arqserial.ReadStringUntil(buffer, terminator); }

void FlowSerialPrint(String& data) { arqserial.WriteString(data); }
void FlowSerialPrint(char data){	arqserial.Print(data);}
void FlowSerialPrint(const char str[]) {	arqserial.Print(str);}
void FlowSerialDebugPrintLn(String& data){	arqserial.DebugPrintLn(data);}
void FlowSerialDebugPrintLn(const char str[]) {	arqserial.DebugPrintLn(str);}
void FlowSerialPrintLn(String& data){	arqserial.PrintLn(data);}
void FlowSerialPrintLn(const char str[]) {	arqserial.PrintLn(str);}
void FlowSerialPrintLn() { arqserial.PrintLn();}

void SetBaudrate() {
	int br = FlowSerialTimedRead();

	delay(200);

	if (br == 1) FlowSerialBegin(300);
	if (br == 2) FlowSerialBegin(1200);
	if (br == 3) FlowSerialBegin(2400);
	if (br == 4) FlowSerialBegin(4800);
	if (br == 5) FlowSerialBegin(9600);
	if (br == 6) FlowSerialBegin(14400);
	if (br == 7) FlowSerialBegin(19200);
	if (br == 8) FlowSerialBegin(28800);
	if (br == 9) FlowSerialBegin(38400);
	if (br == 10) FlowSerialBegin(57600);
	if (br == 11) FlowSerialBegin(115200);
	if (br == 12) FlowSerialBegin(230400);
	if (br == 13) FlowSerialBegin(250000);
	if (br == 14) FlowSerialBegin(1000000);
	if (br == 15) FlowSerialBegin(2000000);
	if (br == 16) FlowSerialBegin(200000);
	if (br == 17) FlowSerialBegin(500000);
}
