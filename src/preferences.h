#include <arduino.h>					// Arduino

uint8_t enablePin = D0;					// Pin where stepper enable signal is connected to
uint8_t mp3Pin = D3;					// Pin where MP3 module is connected to (oneline protocol)
uint8_t runPin = D4;					// Pin where rotation LED is connected to
uint8_t rxPin = D5;						// Pin where RS485 RX signal is connected to
uint8_t txPin = D6;						// Pin where RS485 TX signal is connected to
uint8_t pulsePin = D7;					// Pin where stepper pulse signal is connected to
uint8_t directionPin = D8;				// Pin where stepper direction signal is connected to
float degreesPerStep = 1.8;				// Motor's number of degrees per step
uint16_t microStepsPerStep = 4;			// Drivers number of micro-steps per step
float stepperReduction = 4.0;			// Motor reduction (stepper steps needed to get 1 step on turntable)
bool invertStepper = false;				// Invert stepper rotation
bool adjustPosition = false;			// Adjust rotation (to clear rounding errors)
uint16_t driverMinimalMicroSec = 3;		// Drivers minimal signal duration in micro-seconds
float requiredRPM = 0.5;				// Required motor RPM
bool ledOnWhenHigh = false;             // LED on level (true=high, false=low). Set to false (low) when using D4 internal LED.
bool traceDebug = false;				// Trace debug messages?
bool traceCode = false;					// Trace this code?
bool traceJava = false;					// Trace Java code?
bool enableEncoder = true;				// Enable encoder (else simulate it)
uint8_t maxDelta = 4;					// Maximum percentage delta around reference point click
float encoderOffsetAngle = 0.0;			// Angle of zero degrees position
float imageOffsetAngle = 0.0;			// Angle to rotate image on zero position
bool clockwiseIncrement = false;		// Encoder increases angle when turned clockwise
uint8_t slaveId = 1;					// Slave Id
uint8_t regId = 0;						// Register Id
float radius = 47.0;					// Radius of track marks (and click proximity)
uint8_t trackCount = 1;					// Count of tracks
bool enableSound = false;				// Globally enable sound
bool enableCircles = false;				// Enable tracks circles
uint16_t beforeSoundIndex = 0;			// Before move sound index
float beforeSoundDuration = 0.0;		// Before move sound duration (seconds)
uint8_t beforeSoundVolume = 20;			// Before sound volume (max 30)
uint16_t moveSoundIndex = 0;			// Move sound index
uint8_t moveSoundVolume = 20;			// Move sound volume (max 30)
uint16_t afterSoundIndex = 0;			// After move sound index
float afterSoundDuration = 0.0;			// After move sound duration
uint8_t afterSoundVolume = 20;			// After sound volume (max 30)
#define POSITION_COUNT 36				// Maximum number of tracks
float indexes[POSITION_COUNT+1];		// Angles of each index
float inertiaFactorStarting = 5;		// Divide speed by this number when starting
float inertiaAngleStarting = 6;			// Increase speed over this angle when starting
float inertiaFactorStopping = 5;		// Divide speed up to this number when stopping
float inertiaAngleStopping = 6;			// Start decreasing speed at this angle before target when stopping