#ifndef StepperCommand_h
	#define StepperCommand_h

	#include <inttypes.h>

	#ifdef __cplusplus
		class StepperCommand {
			public:
				StepperCommand(uint8_t _pulsePin, uint8_t _directionPin, uint8_t _enablePin, bool _traceDebug);
				void begin();
				void setParams(float _degreesPerStep, uint16_t _microStepsPerStep, float _stepperReduction, bool _invertStepper, uint8_t _driverMinimalMicroSec, float _RPM, bool _traceDebug);
				void setDirection(int16_t direction);
				void rotateAngle(float _angle);
				unsigned long microStepsForAngle(float _angle);
				float anglePerMicroStep(void);
				uint8_t getDirection(void);
				void turnOneMicroStep(void);

			private:
				float degreesPerStep;						// Motor's number of degrees per step
				float stepDuration;							// Step duration in seconds
				uint16_t microStepsPerStep;					// Drivers number of micro-steps per step
				float stepperReduction;						// Stepper reduction
				uint8_t driverMinimalMicroSec;				// Drivers minimal signal duration in micro-seconds
				float requiredRPM;							// Required motor RPM
				bool invertStepper;							// Invert stepper rotation
				bool traceDebug;							// Trace debug messages?
				uint8_t pulsePin;							// Driver's pulse Pin
				uint8_t directionPin;						// Driver's direction Pin
				uint8_t enablePin;							// Drivers enable Pin
				int8_t currentDirection;					// Current direction (-1, 1)
		};
	#endif
#endif
