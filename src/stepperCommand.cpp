#include <stepperCommand.h>
#include <Arduino.h>

// Class initialization 
//   input:
//       _pulsePin: Pin where drivers pulse is connected to
//       _directionPin: Pin where drivers direction is connected to
//       _enablePin: Pin where drivers enable is connected to (can be zero)
//       _traceDebug: True to trace debug message
StepperCommand::StepperCommand(uint8_t _pulsePin, uint8_t _directionPin, uint8_t _enablePin, bool _traceDebug) {
  // Internal variables
  requiredRPM = 1;                                 // Required stepper RPM
  traceDebug = _traceDebug;                        // Trace debug messages?
  pulsePin = _pulsePin;                            // Driver's pulse Pin
  directionPin = _directionPin;                    // Driver's direction Pin
  enablePin = _enablePin;                          // Driver's enable Pin
}

// Set stepper parameters
//  input:
//       _degreesPerStep: stepper number of degrees per step (usually 1.8)
//       _microStepsPerStep: number of microsteps per step (usually 1, 2, 4, 8...)
//       _stepperReduction: stepper reduction (1 stepper revolution = turntable stepperReduction revolution)
//       _driverMinimalMicroSec: minimum microseconds time driver needs to acquire a signal (around 2-3 uS)
//       _traceDebug: True to trace debug message

void StepperCommand::setParams(float _degreesPerStep, uint16_t _microStepsPerStep, float _stepperReduction,  uint8_t _driverMinimalMicroSec, float _RPM, bool _traceDebug) {
  degreesPerStep = _degreesPerStep;                // stepper's number of degrees per step
  microStepsPerStep = _microStepsPerStep;          // Drivers number of micro-steps per step
  driverMinimalMicroSec = _driverMinimalMicroSec;  // Drivers minimal signal duration in micro-seconds
  requiredRPM = _RPM;                              // Turntable required RPM
  stepperReduction = _stepperReduction;            // Stepper reduction
  traceDebug = _traceDebug;                        // Trace debug messages?

  // Compute steps needed to make 1 full turn
  float stepsPerRotation = 360.0 * microStepsPerStep / degreesPerStep;
  // Compute 1 step duration (in seconds)
  float oneStepDuration = 60 / (_RPM * stepsPerRotation);
  // Check for driver's minimum duration
  if ((oneStepDuration / 2.0) > (driverMinimalMicroSec * 1E6)) {
      Serial.print("*** Step duration is too low, set to minimum ***");
      // Force step duration
      oneStepDuration = driverMinimalMicroSec * 2;
  }
  stepDuration = oneStepDuration;
  if (traceDebug) {
      Serial.printf("Each step will last %.2f ms to make %.1f RPM at %.1f° per step at %d micro-steps with reductor of %.2f\n", oneStepDuration * 1000, _RPM, degreesPerStep, microStepsPerStep, stepperReduction);
  }
}

//  Pin initialization
void StepperCommand::begin(){
  if (enablePin != -1) {
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, 0);
    delayMicroseconds(driverMinimalMicroSec);
  }
  digitalWrite(pulsePin, 0);
  pinMode(pulsePin, OUTPUT);
  setDirection(1);
  pinMode(directionPin, OUTPUT);
}

//  Moves stepper giving specified angle (blocking)
//       input:
//           _angle: angle to rotate stepper, in degrees
void StepperCommand::rotateAngle(float _angle){
  unsigned long microSteps = microStepsForAngle(_angle);
  
  // Rotate stepper the right number of micro-steps
  for (unsigned long i=0; i < microSteps; i++){
    digitalWrite(pulsePin, 1);
    delay(stepDuration * 1E3 / 2.0);
    digitalWrite(pulsePin, 0);
    delay(stepDuration * 1E3 / 2.0);
  }
}

//  Return number of microsteps needed to move a given angle. Set the right direction.
//       input:
//           _angle: angle to rotate stepper, in degrees
//       output:
//           count of micro steps to turn the angle
//       side effect:
//           stepper direction has been set
unsigned long StepperCommand::microStepsForAngle(float _angle){
  unsigned long microSteps = (abs(_angle) + (abs(anglePerMicroStep()) / 2.0)) * microStepsPerStep * stepperReduction / degreesPerStep;
  if (microSteps) {
    if (traceDebug) {
        Serial.printf("Stepper needs %ld micro-steps of %.3f ms to turn about %f° in %f s\n", microSteps, stepDuration * 1000.0, _angle, microSteps * stepDuration);
    }
    // Set correct direction
    if (_angle < 0.0) {
      setDirection(1);
    } else {
      setDirection(-1);
    }
  }
  return microSteps;
}

//  Returns angle corresponding to one micro step
float StepperCommand::anglePerMicroStep(void){
  return currentDirection * degreesPerStep / (microStepsPerStep  * stepperReduction);
}

// Set stepper direction
void StepperCommand::setDirection(int16_t direction) {
  if (direction < 0 ) {
    currentDirection = -1;
    digitalWrite(directionPin, 0);
  } else {
    currentDirection = 1;
    digitalWrite(directionPin, 1);
  }
  delayMicroseconds(driverMinimalMicroSec);
}

// Returns current direction
uint8_t StepperCommand::getDirection(void) {
  return currentDirection;
}

//  Turns stepper one micro step
void StepperCommand::turnOneMicroStep(void){
  digitalWrite(pulsePin, 1);
  delay(stepDuration * 1E3 / 2.0);
  digitalWrite(pulsePin, 0);
  delay(stepDuration * 1E3 / 2.0);
}
