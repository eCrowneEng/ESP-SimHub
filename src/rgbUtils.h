#ifdef INCLUDE_WS2812B
void ReadRGB(int maxLeds, int righttoleft, Adafruit_NeoPixel* neopixels, int ENABLE_BLUETOOTH) {
	ENABLE_BLUETOOTH = 1;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint16_t b1;
	uint16_t b2;

	uint8_t j;
	int mode = 1;
	mode = FlowSerialTimedRead();
	while (mode > 0)
	{
		// Read all
		if (mode == 1) {
			for (j = 0; j < maxLeds; j++) {
				r = FlowSerialTimedRead();
				g = FlowSerialTimedRead();
				b = FlowSerialTimedRead();

				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		// partial led data
		else if (mode == 2) {
			int startled = FlowSerialTimedRead();
			int numleds = FlowSerialTimedRead();

			for (j = startled; j < startled + numleds; j++) {
				/*	if (ENABLE_BLUETOOTH == 0) {*/
				r = FlowSerialTimedRead();
				g = FlowSerialTimedRead();
				b = FlowSerialTimedRead();
				
				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		// repeated led data
		else if (mode == 3) {
			int startled = FlowSerialTimedRead();
			int numleds = FlowSerialTimedRead();

			r = FlowSerialTimedRead();
			g = FlowSerialTimedRead();
			b = FlowSerialTimedRead();

			for (j = startled; j < startled + numleds; j++) {
				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		mode = FlowSerialTimedRead();
	}

	if (maxLeds > 0) {
		neopixels->show();
	}
}
#endif

#ifdef INCLUDE_WS2801
void ReadRGBWS2801(int maxLeds, int righttoleft, Adafruit_WS2801* neopixels, int ENABLE_BLUETOOTH) {
	ENABLE_BLUETOOTH = 1;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint16_t b1;
	uint16_t b2;

	uint8_t j;
	byte mode = 1;
	mode = FlowSerialTimedRead();
	while (mode > 0)
	{
		// Read all
		if (mode == 1) {
			for (j = 0; j < maxLeds; j++) {
				r = FlowSerialTimedRead();
				g = FlowSerialTimedRead();
				b = FlowSerialTimedRead();

				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		// partial led data
		else if (mode == 2) {
			int startled = FlowSerialTimedRead();
			int numleds = FlowSerialTimedRead();

			for (j = startled; j < startled + numleds; j++) {
				/*	if (ENABLE_BLUETOOTH == 0) {*/
				r = FlowSerialTimedRead();
				g = FlowSerialTimedRead();
				b = FlowSerialTimedRead();
			
				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		// repeated led data
		else if (mode == 3) {
			int startled = FlowSerialTimedRead();
			int numleds = FlowSerialTimedRead();

			r = FlowSerialTimedRead();
			g = FlowSerialTimedRead();
			b = FlowSerialTimedRead();

			for (j = startled; j < startled + numleds; j++) {
				if (righttoleft == 1) {
					neopixels->setPixelColor(maxLeds - j - 1, r, g, b);
				}
				else {
					neopixels->setPixelColor(j, r, g, b);
				}
			}
		}

		mode = FlowSerialTimedRead();
	}

	if (maxLeds > 0) {
		neopixels->show();
	}
}
#endif
