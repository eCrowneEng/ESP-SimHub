#include <Arduino.h>

void ConfigureHC05(String name) {
	Serial.begin(38400);

	delay(2000);
	Serial.readString();

	Serial.print("AT+ORGL\r\n");
	delay(500);
	bool res = Serial.readString().startsWith("OK");

	Serial.print("AT+PSWD=1234\r\n");
	delay(500);
	res = Serial.readString().startsWith("OK");

	Serial.print("AT+UART=115200,1,0\r\n");
	delay(500);
	res = Serial.readString().startsWith("OK");

	Serial.println("AT+NAME=" + name + "\r\n");
	delay(500);
	res = res && Serial.readString().startsWith("OK");

	pinMode(13, OUTPUT);
	if (res) {
		while (true) {
			digitalWrite(13, HIGH);
			delay(2000);
			digitalWrite(13, LOW);
			delay(2000);
		}
	}
	else {
		while (true) {
			digitalWrite(13, HIGH);
			delay(250);
			digitalWrite(13, LOW);
			delay(250);
		}
	}
}