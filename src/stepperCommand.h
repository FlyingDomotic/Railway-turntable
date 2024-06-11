#ifndef StepperCommand_h
	#define StepperCommand_h

	#include <inttypes.h>

	#ifdef __cplusplus
		class StepperCommand {
			public:
				enum stepperPhases {						// Rotation phases
					phaseIdle,								// We're idle
					phaseUp,								// Output is high
					phaseDown,								// Output is down
					phaseEnd								// Full phase done (signaled only once then reset to idle)
				};
				StepperCommand(uint8_t _pulsePin, uint8_t _directionPin, uint8_t _enablePin, bool _traceDebug);
				void begin();
				void setParams(float _degreesPerStep, uint16_t _microStepsPerStep, float _stepperReduction, bool _invertStepper, uint8_t _driverMinimalMicroSec, float _RPM, bool _traceDebug);
				void setDirection(int16_t direction);
				void rotateAngle(float _angle);
				unsigned long microStepsForAngle(float _angle);
				float anglePerMicroStep(void);
				uint8_t getDirection(void);
				void turnOneMicroStep(float requiredDuration = 0.0);
				float getStepDuration(void);
				stepperPhases stepperLoop(void);

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
				stepperPhases stepperPhase;					// Current stepper phase
				unsigned long lastPhaseStartUs;				// Time when last phase started (us)
				unsigned long halfStepDurationUs;			// Current half step duration (us)
		};
	#endif
#endif