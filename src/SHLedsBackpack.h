#ifdef INCLUDE_LEDBACKPACK
void ADA7SEG_SetDisplayFromSerial(int idx)
{
	byte displayValues[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	// Digits
	for (int i = 0; i < 8; i++) {
		displayValues[i] = (char)FlowSerialTimedRead();
	}

	for (int i = 0; i < 2; i++) {
		ADA_HT16K33_7SEGMENTS.writeDigitRaw(i, displayValues[i]);
	}

	ADA_HT16K33_7SEGMENTS.writeDigitRaw(2, 0);

	for (int i = 2; i < 4; i++) {
		ADA_HT16K33_7SEGMENTS.writeDigitRaw(i + 1, displayValues[i]);
	}

	ADA_HT16K33_7SEGMENTS.writeDisplay();
}

void ADA_HT16K33BICOLOR_Matrix_Read() {
	int luminosity = FlowSerialTimedRead();

	if (ADA_HT16K33_Matrix_luminosity != luminosity) {
		ADA_HT16K33_MATRIX.setBrightness(luminosity);
		ADA_HT16K33_Matrix_luminosity = luminosity;
	}

	for (int j = 0; j < 8; j++) {
		byte row = FlowSerialTimedRead();
		for (int c = 0; c < 8; c++) {
			ADA_HT16K33_MATRIX.drawPixel(j, c, ((row & (1 << (8 - c - 1))) > 0) ? LED_ON : LED_OFF);
		}
	}

	ADA_HT16K33_MATRIX.writeDisplay();
}
#endif