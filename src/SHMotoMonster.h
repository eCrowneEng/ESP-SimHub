#define SH_BRAKEVCC 0
#define SH_CW   1
#define SH_CCW  2
#define SH_BRAKEGND 3
#define SH_CS_THRESHOLD 100

/*  VNH2SP30 pin definitions
 xxx[0] controls '1' outputs
 xxx[1] controls '2' outputs */
int SHMM_inApin[2] = { 7, 4 };  // INA: Clockwise input
int SHMM_inBpin[2] = { 8, 9 }; // INB: Counter-clockwise input
int SHMM_pwmpin[2] = { 5, 6 }; // PWM input
int SHMM_cspin[2] = { 2, 3 }; // CS: Current sense ANALOG input
int SHMM_enpin[2] = { 0, 1 }; // EN: Status of switches output (Analog pin)

int SHMM_statpin = 13;


// pwmMode - sets the PWM frequency, valid options as follows:
// pwmMode = 1 will use 4kHz PWM, might reduce coil whine for blowers, use heatsinks on the MM - check MM temp at a low fan speed.
// pwmMode = 2 will use 8kHz PWM, might be OK for blowers with active cooling on the MM - check MM temp at a low fan speed.
// pwmMode = 3 will use 31kHz PWM, use with caution - not for blowers with MM as it will cause very high temps. Check MM temp at a low fan speed.
// pwmMode = 4 will use 980Hz PWM, default mode which will work with all fan types, will cause coil whine if using a MM.
// server fans - should be able to use pwmMode = 2 or 3.  If you are using the PWM control on the server fan, leave this at default 0.
// if you have blowers with a monster moto, try pwmMode = 1 or 2 and check whether your monster moto temp at low speeds.
int pwmMode = 0;        // value of 0, 1, 2 or 3 - modes 2 and 3 will overheat a Monster Moto if used with blowers

void setupSHMotoMonster()
{
	pinMode(SHMM_statpin, OUTPUT);
	// Initialize braked
	for (int i = 0; i < 2; i++)
	{
		digitalWrite(SHMM_inApin[i], LOW);
		digitalWrite(SHMM_inBpin[i], LOW);
	}

	// disable timer0's interrupt handler - this will disable Arduino's time keeping functions such as delay()
	if (pwmMode > 0)
	{
		TIMSK0 &= B11111110;
	}
	if (pwmMode == 1)
	{
		// Set pins 5 & 6 to Phase-correct PWM of 3.9 kHz (prescale factor of 8)
		TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00); // phase-correct PWM
		TCCR0B = _BV(CS01);  // prescaler of 8, gives frequency 61kHz/8/2 = 3.9kHz
	}
	else if (pwmMode == 2)
	{
		// Set pins 5 & 6 to Fast PWM of 7.8 kHz (prescale factor of 8)
		TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); // fast PWM
		TCCR0B = _BV(CS01);  // prescaler of 8, gives frequency 61kHz/8 = 7.8kHz
	}
	else if (pwmMode == 3)
	{
		// Set pins 5 & 6 to Phase-correct PWM of 31.25 kHz (prescale factor of 1)
		TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00); // phase-correct PWM
		TCCR0B = _BV(CS00); // prescaler of 1, gives frequency 61kHz/1/2 = 31.25kHz
	}
	else if (pwmMode == 4)
	{
		// Set pins 5 & 6 to Fast PWM of 980Hz (prescale factor of 64)
		TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); // fast PWM
		TCCR0B = _BV(CS01) | _BV(CS00);  // prescaler of 64, gives frequency 61kHz/64 = 980Hz
	}
}

void SHMM_motorOff(int motor)
{

	digitalWrite(SHMM_inApin[motor], LOW);
	digitalWrite(SHMM_inBpin[motor], LOW);
	analogWrite(SHMM_pwmpin[motor], 0);
}

/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.

 motor: this should be either 0 or 1, will selet which of the two
 motors to be controlled

 direct: Should be between 0 and 3, with the following result
 0: Brake to VCC
 1: Clockwise
 2: CounterClockwise
 3: Brake to GND

 pwm: should be a value between ? and 1023, higher the number, the faster
 it'll go
 */
void SHMM_motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{
	if (motor <= 1)
	{
		if (direct <= 4)
		{
			// Set inA[motor]
			if (direct <= 1)
				digitalWrite(SHMM_inApin[motor], HIGH);
			else
				digitalWrite(SHMM_inApin[motor], LOW);

			// Set inB[motor]
			if ((direct == 0) || (direct == 2))
				digitalWrite(SHMM_inBpin[motor], HIGH);
			else
				digitalWrite(SHMM_inBpin[motor], LOW);

			analogWrite(SHMM_pwmpin[motor], pwm);
		}
	}
}