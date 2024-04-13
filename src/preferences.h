uint8_t enablePin = D0;                 // Pin where stepper enable signal is connected to
uint8_t pulsePin = D1;                  // Pin where stepper pulse signal is connected to
uint8_t directionPin = D2;              // Pin where stepper direction signal is connected to
uint8_t rxPin = D5;                     // Pin where RS485 RX signal is connected to
uint8_t txPin = D6;                     // Pin where RS485 TX signal is connected to
uint8_t mp3Pin = D7;                    // Pin where MP3 module is connected to (one line protocol)
uint8_t runPin = D8;                    // Pin where rotation LED is connected to
float degreesPerStep = 1.8;             // Motor's number of degrees per step
uint16_t microStepsPerStep = 2;         // Drivers number of micro-steps per step
float stepperReduction = 1.0;           // Motor reduction (1 stepper revolution = turntable stepperReduction revolution)
bool adjustPosition = false;			// Adjust rotation (to clear rounding errors)
uint16_t driverMinimalMicroSec = 3;     // Drivers minimal signal duration in micro-seconds
float requiredRPM = 2.0;                // Required motor RPM
bool traceDebug = false;                // Trace debug messages?
bool traceCode = true;                  // Trace this code?
bool enableEncoder = true;              // Enable encoder (else simulate it)
uint8_t maxDelta = 4;                   // Maximum percentage delta around reference point click
float encoderOffsetAngle = 0.0;         // Angle of zero degrees position
float imageOffsetAngle = 0.0;           // Angle to rotate image on zero position
uint8_t slaveId = 1;                    // Slave Id
uint8_t regId = 0;                      // Register Id
float radius = 47.0;                    // Radius of track marks (and click proximity)
uint8_t trackCount = 1;                 // Count of tracks
bool enableSound = false;               // Globally enable sound
uint8_t soundVolume = 20;                // Sound volume (max 30)
uint8_t beforeSoundIndex = 0;           // Before move sound index
float beforeSoundDuration = 0.0;        // Before move sound duration (seconds)
uint8_t moveSoundIndex = 0;             // Move sound index
uint8_t afterSoundIndex = 0;            // After move sound index
float afterSoundDuration = 0.0;         // After move sound duration
#define POSITION_COUNT 36               // Maximum number of tracks
float indexes[POSITION_COUNT+1];        // Angles of each index