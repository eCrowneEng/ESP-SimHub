#define MESSAGE_HEADER 0x03

void Command_Hello() {
	FlowSerialTimedRead();
	delay(10);
	FlowSerialPrint(VERSION);
	FlowSerialFlush();
}

void Command_SetBaudrate() {
	SetBaudrate();
}

void Command_ButtonsCount() {
	FlowSerialWrite(ENABLED_BUTTONS_COUNT + ENABLED_BUTTONMATRIX * (BMATRIX_COLS * BMATRIX_ROWS));
	FlowSerialFlush();
}

void Command_TM1638Count() {
	FlowSerialWrite((byte)(TM1638_ENABLEDMODULES));
	FlowSerialFlush();
}

void Command_SimpleModulesCount() {
	FlowSerialWrite(MAX7221_ENABLEDMODULES + TM1637_ENABLEDMODULES + TM1637_6D_ENABLEDMODULES + ENABLE_ADA_HT16K33_7SEGMENTS);
	FlowSerialFlush();
}

void Command_Acq() {
	FlowSerialWrite(0x03);
	FlowSerialFlush();
}

void Command_DeviceName() {
	FlowSerialPrint(DEVICE_NAME);
	FlowSerialPrint("\n");
	FlowSerialFlush();
}

void Command_UniqueId() {
	auto id = getUniqueId();
	FlowSerialPrint(id);
	FlowSerialPrint("\n");
	FlowSerialFlush();
}

void Command_MCUType() {
	FlowSerialPrint(SIGNATURE_0);
	FlowSerialPrint(SIGNATURE_1);
	FlowSerialPrint(SIGNATURE_2);
	FlowSerialFlush();
}

void Command_EncodersCount() {
#ifdef INCLUDE_ENCODERS
	FlowSerialWrite(ENABLED_ENCODERS_COUNT);
#endif
}

void Command_SpeedoData() {
#ifdef INCLUDE_SPEEDOGAUGE
	speedoTonePin.readFromString();
#endif
}

void Command_TachData() {
#ifdef INCLUDE_TACHOMETER
	rpmTonePin.readFromString();
#endif
}

void Command_BoostData() {
#ifdef INCLUDE_BOOSTGAUGE
	shBOOSTPIN.readFromString();
#endif
}

void Command_TempData() {
#ifdef INCLUDE_TEMPGAUGE
	shTEMPPIN.readFromString();
#endif
}

void Command_ConsData() {
#ifdef INCLUDE_CONSGAUGE
	shCONSPIN.readFromString();
#endif
}

void Command_FuelData() {
#ifdef INCLUDE_FUELGAUGE
	shFUELPIN.readFromString();
#endif
}

void Command_GLCDData() {
#ifdef INCLUDE_OLED
	shGLCD.read();
#endif 
#ifdef INCLUDE_NOKIALCD
	shNOKIA.read();
#endif 
}

void Command_ExpandedCommandsList() {
#ifdef INCLUDE_SPEEDOGAUGE
	FlowSerialPrintLn("speedo");
#endif
#ifdef INCLUDE_TACHOMETER
	FlowSerialPrintLn("tachometer");
#endif
#ifdef INCLUDE_BOOSTGAUGE
	FlowSerialPrintLn("boostgauge");
#endif
#ifdef INCLUDE_TEMPGAUGE
	FlowSerialPrintLn("tempgauge");
#endif
#ifdef INCLUDE_FUELGAUGE
	FlowSerialPrintLn("fuelgauge");
#endif
#ifdef INCLUDE_CONSGAUGE
	FlowSerialPrintLn("consumptiongauge");
#endif
#ifdef INCLUDE_DM163_MATRIX
	FlowSerialPrintLn("dm163rgb");
#endif
#ifdef INCLUDE_ENCODERS
	FlowSerialPrintLn("encoders");
#endif
	FlowSerialPrintLn("mcutype");
	FlowSerialPrintLn("keepalive");
	FlowSerialPrintLn();
	FlowSerialFlush();
}

void Command_TM1638Data() {
#ifdef INCLUDE_TM1638
	// TM1638
	for (int j = 0; j < TM1638_ENABLEDMODULES; j++) {
		// Wait for display data
		int newIntensity = FlowSerialTimedRead();
		if (newIntensity != TM1638_screens[j]->Intensity) {
			TM1638_screens[j]->Screen->setupDisplay(true, newIntensity);
			TM1638_screens[j]->Intensity = newIntensity;
		}

		TM1638_SetDisplayFromSerial(TM1638_screens[j]);
	}
#endif
}

void Command_Features() {
	delay(10);

	// Matrix
	if (MAX7221_MATRIX_ENABLED == 1 || ENABLE_ADA_HT16K33_BiColorMatrix == 1 || ENABLE_ADA_HT16K33_SingleColorMatrix == 1) {
		FlowSerialPrint("M");
	}

	// LCD
	if (I2CLCD_enabled == 1) {
#ifdef INCLUDE_I2CLCD
		FlowSerialPrint("L");
#endif
	}

	if (ENABLED_NOKIALCD > 0 || ENABLED_OLEDLCD > 0) {
		FlowSerialPrint("K");
	}

	// Gear
	FlowSerialPrint("G");

	// Name
	FlowSerialPrint("N");

	// UniqueID
	FlowSerialPrint("I");

	// Additional buttons
	FlowSerialPrint("J");

	// Custom Protocol Support
	FlowSerialPrint("P");

	// Xpanded support
	FlowSerialPrint("X");

	// RGB MATRIX
#if defined(INCLUDE_WS2812B_MATRIX) || defined(INCLUDE_DM163_MATRIX) || defined(INCLUDE_SUNFOUNDERSH104P_MATRIX)
	if (WS2812B_MATRIX_ENABLED > 0 || DM163_MATRIX_ENABLED > 0 || SUNFOUNDERSH104P_MATRIX_ENABLED >0) {
		FlowSerialPrint("R");
	}
#endif

#if defined(INCLUDE_SHAKEIDUALVNH5019) || defined(INCLUDE_SHAKEITADASHIELD) || defined(INCLUDE_SHAKEITDKSHIELD) || defined(INCLUDE_SHAKEITL298N) || defined(INCLUDE_SHAKEITMOTOMONSTER) || defined(INCLUDE_SHAKEITPWM)  || defined(INCLUDE_SHAKEITPWMFANS)
	FlowSerialPrint("V");
#endif 

	FlowSerialPrint("\n");
	FlowSerialFlush();
}

void Command_Motors() {
#if defined(INCLUDE_SHAKEIDUALVNH5019) || defined(INCLUDE_SHAKEITADASHIELD) || defined(INCLUDE_SHAKEITDKSHIELD) || defined(INCLUDE_SHAKEITL298N) || defined(INCLUDE_SHAKEITMOTOMONSTER) || defined(INCLUDE_SHAKEITPWM) || defined(INCLUDE_SHAKEITPWMFANS)
	String buffer = "";
	char action = FlowSerialTimedRead();
	// Count
	if (action == 'C') {
		FlowSerialWrite(255);
		FlowSerialWrite(min(1, DUALVNH5019_ENABLED) * 2 + ADAMOTORS_SHIELDSCOUNT * 4 + min(1, DKMOTOR_SHIELDSCOUNT) * 4 + min(1, L98NMOTORS_ENABLED) * 2 + min(1, MOTOMONSTER_ENABLED) * 2 + min(4, SHAKEITPWM_ENABLED_MOTORS) + min(4, SHAKEITPWMFANS_ENABLED_MOTORS));

#ifdef  INCLUDE_SHAKEITADASHIELD
		buffer = shShakeitAdaMotorShieldV2.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef  INCLUDE_SHAKEITDKSHIELD
		buffer = shShakeitDKMotorShield.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef  INCLUDE_SHAKEITL298N
		buffer = shShakeitL298N.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef INCLUDE_SHAKEITMOTOMONSTER
		buffer = shShakeitMotoMonster.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef INCLUDE_SHAKEIDUALVNH5019
		buffer = shShakeitDualVNH5019.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef INCLUDE_SHAKEITPWM
		buffer = shShakeitPWM.providerName() + ";";
		FlowSerialPrint(buffer);
#endif
#ifdef INCLUDE_SHAKEITPWMFANS
		buffer = shShakeitPWMFans.providerName() + ";";		
		FlowSerialPrint(buffer);
#endif
		FlowSerialPrintLn();
	}
	// Set motors
	else if (action == 'S') {
#ifdef  INCLUDE_SHAKEITADASHIELD
		shShakeitAdaMotorShieldV2.read();
#endif
#ifdef  INCLUDE_SHAKEITDKSHIELD
		shShakeitDKMotorShield.read();
#endif
#ifdef  INCLUDE_SHAKEITL298N
		shShakeitL298N.read();
#endif
#ifdef INCLUDE_SHAKEITMOTOMONSTER
		shShakeitMotoMonster.read();
#endif
#ifdef INCLUDE_SHAKEIDUALVNH5019
		shShakeitDualVNH5019.read();
#endif
#ifdef INCLUDE_SHAKEITPWM
		shShakeitPWM.read();
#endif
#ifdef INCLUDE_SHAKEITPWMFANS
		shShakeitPWMFans.read();
#endif
	}
#endif
}

void Command_Shutdown() {
#if	defined(INCLUDE_DM163_MATRIX)
	if (DM163_MATRIX_ENABLED > 0) {
		shRGBLedsDM163.clear();
	}
#endif
#if	defined(INCLUDE_SUNFOUNDERSH104P_MATRIX)
	if (SUNFOUNDERSH104P_MATRIX_ENABLED > 0) {
		shRGBMatrixSunfounderSH104P.clear();
	}
#endif
#if defined(INCLUDE_WS2812B_MATRIX) 
	if (WS2812B_MATRIX_ENABLED > 0) {
		shRGBMatrixWS2812B.clear();
	}
#endif
}

void Command_7SegmentsData() {
#ifdef INCLUDE_TM1637
	// TM1637
	for (int j = 0; j < TM1637_ENABLEDMODULES; j++) {
		// Intensity
		TM1637_screens[j]->setBrightness(FlowSerialTimedRead());
		TM1637_SetDisplayFromSerial(TM1637_screens[j]);
	}
#endif

#ifdef INCLUDE_TM1637_6D
	// TM1637
	for (int j = 0; j < TM1637_6D_ENABLEDMODULES; j++) {
		// Intensity
		TM1637_6D_screens[j]->set(FlowSerialTimedRead());
		TM1637_6D_SetDisplayFromSerial(TM1637_6D_screens[j]);
	}
#endif

	// MAX7221
#ifdef INCLUDE_MAX7221_MODULES
	shMAX72217Segment.read();
#endif // INCLUDE_MAX7221_MODULES

#ifdef INCLUDE_LEDBACKPACK
	// Simple ADA display
	for (int j = 0; j < ENABLE_ADA_HT16K33_7SEGMENTS; j++) {
		int newIntensity = FlowSerialTimedRead();
		ADA_HT16K33_7SEGMENTS.setBrightness(newIntensity * 2 + 1);

		ADA7SEG_SetDisplayFromSerial(j);
	}
#endif
}

void Command_RGBLEDSCount() {
	FlowSerialWrite(
		(byte)(
			WS2812B_RGBLEDCOUNT + 
			PL9823_RGBLEDCOUNT +
			WS2801_RGBLEDCOUNT
#ifdef INCLUDE_RGB_LEDS_NEOPIXELBUS
	+ neoPixelBusCount()
#endif
			));
	FlowSerialFlush();
}

void Command_RGBLEDSData()
{
#ifdef INCLUDE_WS2812B
	shRGBLedsWS2812B.read();
#endif
#ifdef INCLUDE_PL9823
	shRGBLedsPL9823.read();
#endif
#ifdef INCLUDE_WS2801
	shRGBLedsWS2801.read();
#endif
#ifdef INCLUDE_RGB_LEDS_NEOPIXELBUS
	neoPixelBusRead();
#endif
#ifdef INCLUDE_WS2812B
	shRGBLedsWS2812B.show();
#endif
#ifdef INCLUDE_WS2801
	shRGBLedsWS2801.show();
#endif
#ifdef INCLUDE_RGB_LEDS_NEOPIXELBUS
	neoPixelBusShow();
#endif
	// Acq !
	FlowSerialWrite(0x15);
}

void Command_RGBMatrixData() {
#if	defined(INCLUDE_DM163_MATRIX)
	if (DM163_MATRIX_ENABLED > 0) {
		shRGBLedsDM163.read();
	}
#endif

#if defined(INCLUDE_WS2812B_MATRIX) 
	if (WS2812B_MATRIX_ENABLED > 0) {
		shRGBMatrixWS2812B.read();
	}
#endif

#if	defined(INCLUDE_SUNFOUNDERSH104P_MATRIX)
	if (SUNFOUNDERSH104P_MATRIX_ENABLED > 0) {
		shRGBMatrixSunfounderSH104P.read(); 
	}
#endif
	// Acq !
	FlowSerialWrite(0x15);
}

void Command_MatrixData() {
#ifdef INCLUDE_MAX7221MATRIX
	shMatrixMAX7219.read();
#endif // INCLUDE_MAX7221MATRIX

#ifdef INCLUDE_LEDBACKPACK
	if (ENABLE_ADA_HT16K33_BiColorMatrix == 1) {
		ADA_HT16K33BICOLOR_Matrix_Read();
	}
#endif

#ifdef INCLUDE_HT16K33_SINGLECOLORMATRIX
	shMatrixHT16H33SingleColor.read();
#endif
}

void Command_GearData() {
	char gear = FlowSerialTimedRead();

#ifdef INCLUDE_74HC595_GEAR_DISPLAY
	if (ENABLE_74HC595_GEAR_DISPLAY == 1) {
		RS_74HC595_SetChar(gear);
	}
#endif

#ifdef INCLUDE_6c595_GEAR_DISPLAY
	if (ENABLE_6C595_GEAR_DISPLAY == 1) {
		RS_6c595_SetChar(gear);
	}
#endif
}

void Command_I2CLCDData() {
#ifdef INCLUDE_I2CLCD
	shI2CLcd.read();
#endif
}

void Command_CustomProtocolData() {
	shCustomProtocol.read();
	FlowSerialWrite(0x15);
}
