/*
 *	   English: Railway turntable control
 *	   Français : Contrôle d'un pont tournant férroviaire miniature
 *
 *	Hardware
 *		ESP8266
 *		Stepper with driver (enable/pulse/direction) (for example NEMA 24 with DM556T)
 *		RS485 aboslute encoder (for example QY3806-485RTU)
 *		Serial/RS485 adapter
 *		MP3 reader with XY-V17B chip (optional)
 *		Rotation LED (optional)
 *
 *	Matériel
 *		ESP8266
 *		Moteur pas à pas avec driver (enable/pulse/direction) (par exemple NEMA 24 avec DM556T)
 *		Encodeur absolu RS485 (par exemple QY3806-485RTU)
 *		Adaptateur série/RS485
 *		Lecteur MP3 avec chip XY-V17B (optionnel)
 *		LED de rotation (optionnel)
 *
 *	Connections
 *		By default, the following pins are defined in preference.h file:
 *			D0 (enablePin) : Stepper enable signal
 *			D1 (pulsePin) : Stepper pulse signal
 * 			D2 (directionPin) : Stepper direction signal
 *			D3 (runPin) : Set to high during rotation (LED command)
 *			D4 (mp3Pin) : MP3 module communication line (one line)
 *			D5 (rxPin) : RS485 RX pin
 *			D6 (txPin) : RS485 TX pin
 *
 *	Connexions
 *		Par défaut, les pinoches suivantes sont définies dans le fichier preference.h :
 *			D0 (enablePin) : Signal enable du moteur
 *			D1 (pulsePin) : Signal pulse du moteur
 * 			D2 (directionPin) : Signal direction du moteur
 *			D5 (rxPin) : RX du module RS485
 *			D6 (txPin) : TX du module RS485
 *			D7 (mp3Pin) : Ligne de communication (one line) du module MP3
 *			D8 (runPin) : Haut pendant la rotation (commande LED)
 *
 *	Available URL
 *		/			Root page (displays turntable allowing clicking on tracks)
 *		/status		Returns status in JSON format
 *		/goto/<nnn>	Rurns turntable in front of track <nnn>
 *		/setup		Display setup page
 *		/edit		Manage and edit flie system
 *		/settings	Returns settings in JSON format
 *		/debug		Display internal variables to debug
 *
 *
 * 	URL disponibles
 *		/			Page d'accueil (affiche le pont et permet de cliquer sur les voies)
 *		/status		Retourne l'état sous forme JSON
 *		/goto/<nnn>	Tourne le pont en face de la voie <nnn>
 *		/setup		Affiche la page de configuration
 *		/edit		Gère et édite le système de fichier
 *		/settings	Retourne la configuration au format JSON
 *		/debug		Affiche les variables internes pour déverminer
 *
 *	V1.0.0 FF - Mars 2024 - For Fablab/Pour le FabLab
 *
 *	GNU GENERAL PUBLIC LICENSE - Version 3, 29 June 2007
 *
 */

#include <ESP8266WiFi.h>											// Wifi
#include <ESPAsyncWebServer.h>										// Asynchronous web server
#include <ArduinoOTA.h>												// OTA (network update)
#include <arduino.h>												// Arduino
#include <preferences.h>											// Turn table preferences
#include <stepperCommand.h>											// Stepper
#include <rotationSensor.h>											// Encoder
#include <mp3_player_module_wire.h>									// MP3 player
#include <cmath>													// Math functions
#include <LittleFS.h>												// Flash dile system
#include <littleFsEditor.h>											// LittleFS file system editor
#include <ArduinoJson.h>											// JSON documents

//	---- Variables ----

//	Asynchronous web server
AsyncWebServer webServer(80);										// Web server on port 80
AsyncEventSource events("/events");									// Event root

//	Wifi and module names
String ssid;
String pwd;
String espName = "PontTournant";

//	Preferences file name
#define SETTINGS_FILE "settings.json"

//	Stepper
StepperCommand
	stepper(pulsePin, directionPin, enablePin, traceDebug);			// Setpper class
float lastAngleSent = 0;											// Last sent angle
float currentAngle = 0;												// Current angle
float requiredAngle = -1;											// Required angle
unsigned long microStepsToGo = 0;									// Remaining micro steps to send
enum stepperState {													// Rotation states
	stepperStopped,
	stepperStarting,
	stepperRunning,
	stepperStopping,
	stepperFixing
};
stepperState rotationState = stepperStopped;						// Rotation current state
#define MIN_DELTA_ANGLE 0.02										// Minimum delta before sending angle change
const char stepperStateText0[] PROGMEM = "Stopped";
const char stepperStateText1[] PROGMEM = "Starting";
const char stepperStateText2[] PROGMEM = "Running";
const char stepperStateText3[] PROGMEM = "Stopping";
const char stepperStateText4[] PROGMEM = "Fixing";
const char *const stepperStateTable[] PROGMEM = {stepperStateText0, stepperStateText1, stepperStateText2, stepperStateText3, stepperStateText4};


//	Encodeur
RotationSensor encoder(rxPin, txPin, traceDebug);					// Encoder class
uint16_t resultValue;												// Encoder last result
unsigned long resultValueTime = 0;									// Encoder last result time
bool encoderReading = false;										// Is encoder reading?
unsigned long encoderReadTime = 0;									// Save encoder read time
Modbus::ResultCode encoderLastReadError;							// Last error returned by encoder

// Lecteur MP3
Mp3PlayerModuleWire player(mp3Pin);									// MP3 reader class
unsigned long playerStartedTime = 0;								// Last play time
unsigned long playerDuration = 0;									// Play duration time

//	---- Print rountines ----

static void printInfo(const char* _format, ...) {
	// Make the message
	char msg[512];                              		// Message buffer (message truncated if longer than this)
	va_list arguments;                                  // Variable argument list
	va_start(arguments, _format);                       // Read arguments after _format
	vsnprintf_P(msg, sizeof(msg), _format, arguments);	// Return buffer containing requested format with given arguments
	va_end(arguments);                                  // End of argument list
	Serial.print(msg);			                        // Print on SERIAL_PORT
	events.send(msg, "info");							// Send message to destination
}

static void printError(const char* _format, ...) {
	// Make the message
	char msg[512];                              		// Message buffer (message truncated if longer than this)
	va_list arguments;                                  // Variable argument list
	va_start(arguments, _format);                       // Read arguments after _format
	vsnprintf_P(msg, sizeof(msg), _format, arguments);	// Return buffer containing requested format with given arguments
	va_end(arguments);                                  // End of argument list
	Serial.print(msg);			                        // Print on SERIAL_PORT
	events.send(msg, "error");							// Send message to destination
}

//	---- Preferences routines ----

// Dumps all settings on screen
void dumpSettings(void) {
	printInfo("ssid = %s\n", ssid.c_str());
	printInfo("pwd = %s\n", pwd.c_str());
	printInfo("name = %s\n", espName.c_str());
	printInfo("degreesPerStep = %.2f\n", degreesPerStep);
	printInfo("microStepsPerStep = %d\n", microStepsPerStep);
	printInfo("invertStepper = %s\n", invertStepper ? "false" : "true");
	printInfo("stepperReduction = %.2f\n", stepperReduction);
	printInfo("driverMinimalMicroSec = %d\n", driverMinimalMicroSec);
	printInfo("requiredRPM = %.2f\n", requiredRPM);
	printInfo("maxDelta = %d\n", maxDelta);
	printInfo("encoderOffsetAngle = %.2f\n", encoderOffsetAngle);
	printInfo("clockwiseIncrement = %s\n", clockwiseIncrement ? "true" : "false");
	printInfo("imageOffsetAngle = %.2f\n", imageOffsetAngle);
	printInfo("slaveId = %d\n", slaveId);
	printInfo("regId = %d\n", regId);
	printInfo("radius = %f\n", radius);
	printInfo("enableSound = %s\n", enableSound ? "true" : "false");
	printInfo("soundVolume = %d\n", soundVolume);
	printInfo("beforeSoundIndex = %d\n", beforeSoundIndex);
	printInfo("beforeSoundDuration = %.2f\n", beforeSoundDuration);
	printInfo("moveSoundIndex = %d\n", moveSoundIndex);
	printInfo("afterSoundIndex = %d\n", afterSoundIndex);
	printInfo("afterSoundDuration = %.2f\n", afterSoundDuration);
	printInfo("trackCount = %d\n", trackCount);
	printInfo("traceDebug = %s\n", traceDebug ? "true" : "false");
	printInfo("traceCode = %s\n", traceCode ? "true" : "false");
	printInfo("traceJava = %s\n", traceJava ? "true" : "false");

	// Dump all defined angles
	char name[10];
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		printInfo("%s = %.2f\n", name, indexes[i]);
	}
}

// Read settings
bool readSettings(void) {
	File settingsFile = LittleFS.open(SETTINGS_FILE, "r");			// Open settings file
	if (!settingsFile) {											// Error opening?
		printError("Failed to open config file\n");
		return false;
	}

	JsonDocument settings;
	auto error = deserializeJson(settings, settingsFile);			// Read settings
	settingsFile.close();											// Close file
	if (error) {													// Error reading JSON?
		printError("Failed to parse config file\n");
		return false;
	}

	// Load all settings into corresponding variables
	degreesPerStep = settings["degreesPerStep"].as<float>();
	if (degreesPerStep <= 0.0) {
		printError("degreesPerStep is %f, should be > 0\n", degreesPerStep);
	}
	microStepsPerStep = settings["microStepsPerStep"].as<uint16_t>();
	if (microStepsPerStep == 0) {
		printError("microStepsPerStep is %d, should be > 0\n", microStepsPerStep);
	}
	invertStepper = settings["invertStepper"].as<bool>();
	stepperReduction = settings["stepperReduction"].as<float>();
	if (stepperReduction <= 0.0) {
		printError("stepperReduction is %f, should be > 0\n", stepperReduction);
	}
	driverMinimalMicroSec =settings["driverMinimalMicroSec"].as<uint16_t>();
	requiredRPM = settings["requiredRPM"].as<float>();
	if (requiredRPM <= 0.0) {
		printError("requiredRPM is %f, should be > 0\n", requiredRPM);
	}
	maxDelta = settings["maxDelta"].as<uint8_t>();
	enableEncoder = settings["enableEncoder"].as<bool>();
	adjustPosition = settings["adjustPosition"].as<bool>();
	encoderOffsetAngle = settings["encoderOffsetAngle"].as<float>();
	clockwiseIncrement = settings["clockwiseIncrement"].as<bool>();
	imageOffsetAngle = settings["imageOffsetAngle"].as<float>();
	slaveId = settings["slaveId"].as<uint8_t>();
	regId = settings["regId"].as<uint8_t>();
	radius = settings["radius"].as<float>();
	trackCount = settings["trackCount"].as<uint8_t>();
	traceDebug = settings["traceDebug"].as<bool>();
	traceCode = settings["traceCode"].as<bool>();
	traceJava = settings["traceJava"].as<bool>();
	ssid = settings["ssid"].as<String>();
	pwd = settings["pwd"].as<String>();
	espName = settings["name"].as<String>();
	enableSound = settings["enableSound"].as<bool>();
	soundVolume = settings["soundVolume"].as<uint8_t>();
	beforeSoundIndex = settings["beforeSoundIndex"].as<uint8_t>();
	beforeSoundDuration = settings["beforeSoundDuration"].as<float>();
	moveSoundIndex = settings["moveSoundIndex"].as<uint8_t>();
	afterSoundIndex = settings["afterSoundIndex"].as<uint8_t>();
	afterSoundDuration = settings["afterSoundDuration"].as<float>();

	// Read "axx" variables into a table
	char name[10];
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		indexes[i] = settings[name].as<float>();
	}
	// Dump settings on screen
	dumpSettings();
	// Send values to stepper, encoder and player
	stepper.setParams(degreesPerStep, microStepsPerStep, stepperReduction, invertStepper, driverMinimalMicroSec, requiredRPM, traceDebug);
	encoder.setParams(encoderOffsetAngle, clockwiseIncrement, traceDebug);
	player.set_volume(soundVolume);
	return true;
}

// Write settings
void writeSettings(void) {
	JsonDocument settings;
	char name[10];

	// Load settings in JSON
	settings["ssid"] = ssid.c_str();
	settings["pwd"] = pwd.c_str();
	settings["name"] = espName.c_str();
	settings["degreesPerStep"] = degreesPerStep;
	settings["microStepsPerStep"] = microStepsPerStep;
	settings["driverMinimalMicroSec"] = driverMinimalMicroSec;
	settings["invertStepper"] = invertStepper;
	settings["stepperReduction"] = stepperReduction;
	settings["requiredRPM"] = requiredRPM;
	settings["maxDelta"] = maxDelta;
	settings["enableEncoder"] = enableEncoder;
	settings["adjustPosition"] = adjustPosition;
	settings["encoderOffsetAngle"] = encoderOffsetAngle;
	settings["clockwiseIncrement"] = clockwiseIncrement;
	settings["imageOffsetAngle"] = imageOffsetAngle;
	settings["slaveId"] = slaveId;
	settings["regId"] = regId;
	settings["radius"] = radius;
	settings["enableSound"] = enableSound;
	settings["soundVolume"] = soundVolume;
	settings["beforeSoundIndex"] = beforeSoundIndex;
	settings["beforeSoundDuration"] = beforeSoundDuration;
	settings["moveSoundIndex"] = moveSoundIndex;
	settings["afterSoundIndex"] = afterSoundIndex;
	settings["afterSoundDuration"] = afterSoundDuration;
	settings["traceDebug"] = traceDebug;
	settings["traceCode"] = traceCode;
	settings["traceJava"] = traceJava;
	settings["trackCount"] = trackCount;
	// Load all "axx" variables
	for (uint8_t i = 1; i <= POSITION_COUNT; i++) {
		snprintf(name, sizeof(name), "a%d", i);
		settings[name] = indexes[i];
	}

	File settingsFile = LittleFS.open(SETTINGS_FILE, "w");			// Open settings file
	if (!settingsFile) {											// Error opening?
		printError("Can't open for write settings file\n");
	}
	uint16_t bytes = serializeJsonPretty(settings, settingsFile);	// Write JSON structure to file
	if (!bytes) {													// Error writting?
		printError("Can't write settings file\n");
	}
	settingsFile.flush();											// Flush file
	settingsFile.close();											// Close it
	// Reload variables into stepper, encoder and player
	stepper.setParams(degreesPerStep, microStepsPerStep, stepperReduction, invertStepper, driverMinimalMicroSec, requiredRPM, traceDebug);
	encoder.setParams(encoderOffsetAngle, clockwiseIncrement, traceDebug);
	player.set_volume(soundVolume);
	if (traceCode) {
		printInfo("Sending settings event\n");
	}
	events.send("Ok", "settings");									// Send a "settings" (changed) event
}

//	---- Encoder routines ----

// called at end of Modbus transaction
Modbus::ResultCode modbusCb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
	if (event != encoderLastReadError) {							// Is return code different of last one?
	printError("Read error is now: 0x%x\n", event);
	encoderLastReadError = event;
	}
	if (event == Modbus::EX_SUCCESS) {								// We read succesfully
		currentAngle = encoder.computeAngle(resultValue);			// Load current angle from last result
		encoderReading = false;										// Encoder is not anymore reading
	}
	return Modbus::EX_SUCCESS;
}

//	---- Stepper routines ----

// Called at rotation begining
void startRotation(void) {
	digitalWrite(runPin, HIGH);										// Set rotation pin to high
	if (rotationState == stepperStopped) {							// Are we stopped?
		rotationState = stepperRunning;								// Set to running by default
		if (beforeSoundIndex) {										// Do we have a starting sound?
			rotationState = stepperStarting;						// Set to starting
			playerDuration = beforeSoundDuration * 1000.0;			// Convert duration to ms
			playIndex(beforeSoundIndex);							// Play sound
		} else {
			if (moveSoundIndex) {									// Do we have a rotation sound?
				playIndex(moveSoundIndex);							// Play sound
			} else {
				playStop();											// Stop starting sound
			}
		}
	} else if (rotationState == stepperStopping) {					// Are we stopping?
		rotationState = stepperRunning;								// Set to running again
		if (moveSoundIndex) {										// Do we have a rotation sound?
			playIndex(moveSoundIndex);								// Play sound
		}
	}
}

// Called at rotation end
void stopRotation(void) {
	if (afterSoundIndex) {											// Do we have a stop sound?
		rotationState = stepperStopping;							// Set to stopping
		playerDuration = afterSoundDuration * 1000.0;				// Convert duration in ms
		playIndex(afterSoundIndex);									// Play stopping sound
		return;
	}
	playStop();														// Stop running sound
	rotationState = stepperStopped;									// Force to stopped
	digitalWrite(runPin, LOW);										// Put rotation pin to low
}

// Move turntable to a given track
void moveToTrack(uint8_t index) {
	requiredAngle = indexes[index];									// User's required angle
	float deltaAngle = requiredAngle - currentAngle;				// Difference to move
	if (deltaAngle > 180.0) {										// Set angle into -180/+180 range
		deltaAngle -= 360.0;
	}
	if (deltaAngle < -180.0) {
		deltaAngle += 360.0;
	}
	microStepsToGo = stepper.microStepsForAngle(deltaAngle);		// Compute micro steps to move
	if (microStepsToGo) {											// Do we have to move?
		startRotation();											// Start rotation phase
	}
	if (traceCode) {
		printInfo("Current angle = %.2f, required angle = %.2f, delta angle = %.2f, micro step count = %ld, angle per micro step = %.2f\n", currentAngle, requiredAngle, deltaAngle, microStepsToGo, stepper.anglePerMicroStep());
	}
}

// Ajdust stepper position if required (to be called when stepper is not running, after currentAngle set by encoder)
void fixPosition(void){
	if (adjustPosition && rotationState == stepperStopped) {		// Only if position adjustment is required and stepper stopped
		if (requiredAngle != -1) {									// Don't adjust position after manual move (no resuired angle)
			float deltaAngle = requiredAngle - currentAngle;		// Difference to move
			if (deltaAngle > 180.0) {								// Set angle into -180/+180 range
				deltaAngle -= 360.0;
			}
			if (deltaAngle < -180.0) {
				deltaAngle += 360.0;
			}
			microStepsToGo = stepper.microStepsForAngle(deltaAngle);// Compute micro steps to move
			if (microStepsToGo) {									// Do we have to move?
				rotationState = stepperFixing;						// Set proper state
				if (traceCode) {
					printInfo("Fix: Current angle = %.2f, required angle = %.2f, delta angle = %.2f, micro step count = %ld, angle per micro step = %.2f\n", currentAngle, requiredAngle, deltaAngle, microStepsToGo, stepper.anglePerMicroStep());
				}
			}
		}
	}
}

//	---- MP3 player routines ----

// Play a given index
void playIndex(uint8_t index) {
	playerStartedTime = millis();									// Load starting time
	if (enableSound) {												// If sound enabled
		if (traceCode) {
			printInfo("Playing index %d\n", index);
		}
		player.set_track_index(index);								// Set file index
		player.play();												// Start playing file
	}
}

// Stop playing
void playStop() {
	if (enableSound) {												// If sound enabled
		if (traceCode) {
			printInfo("Stop playing\n");
		}
		player.stop();
	}
}

//	---- Web server routines ----

// Called when /setup is received
void setupReceived(AsyncWebServerRequest *request) {
	AsyncWebServerResponse *response = request->beginResponse(LittleFS, "setup.htm", "text/html");
	request->send(response);										// Send setup.htm
}

// Called when /setttings is received
void settingsReceived(AsyncWebServerRequest *request) {
	AsyncWebServerResponse *response = request->beginResponse(LittleFS, SETTINGS_FILE, "application/json");
	request->send(response);										// Send settings.json
}

// Called when /debug is received
void debugReceived(AsyncWebServerRequest *request) {
	// Send a json document with interresting variables
	JsonDocument answer;
	char text[50];
	answer["lastAngleSent"] = lastAngleSent;
	answer["currentAngle"] = currentAngle;
	answer["requiredAngle"] = requiredAngle;
	answer["microStepsToGo"] = microStepsToGo;
	answer["rotationState"] = rotationState;
	if (rotationState < 0 || rotationState >= sizeof(stepperStateTable)) {
		snprintf_P(text, sizeof(text), PSTR("??? %d ???"), rotationState);
	} else {
		strncpy_P(text, (char *)pgm_read_ptr(&(stepperStateTable[rotationState])), sizeof(text));
	}
	answer["rotationStateText"] = text;
	answer["encoderReading"] = encoderReading;
	answer["encoderReadTime"] = millis() - encoderReadTime;
	answer["encoderLastReadError"] = encoderLastReadError;
	answer["resultValue"] = resultValue;
	answer["resultValueTime"] = millis() - resultValueTime;
	answer["playerStartedTime"] = millis() - playerStartedTime;
	answer["playerDuration"] = playerDuration;
	char buffer[1024];
	serializeJsonPretty(answer, buffer, sizeof(buffer));
	request->send(200, "application/json", String(buffer));
}


// Called when a /status click is received
void statusReceived(AsyncWebServerRequest *request) {
	// Send a json document with data correponding to current position
	float closestDelta = 999.0;
	float closestTrack = -1;
	// Loop through all used tracks
	for (uint8_t i = 1; i <= trackCount; i++) {
		float delta = currentAngle - indexes[i];					// Compute delta between current angle and track given angle
		if (delta < 0.0) {
			delta = - delta;										// Make it positive
		}
		if (delta < closestDelta) {									// Closer than closest?
			closestDelta = delta;									// Load closest delta
			closestTrack = i;										//   and track
		}
	}
	JsonDocument answer;
	switch (rotationState) {
		case stepperStopped:
			answer["rotationState"] = "stopped";
			break;
		case stepperStarting:
			answer["rotationState"] = "starting";
			break;
		case stepperRunning:
			answer["rotationState"] = "running";
			break;
		case stepperStopping:
			answer["rotationState"] = "stopping";
			break;
		case stepperFixing:
			answer["rotationState"] = "fixing";
			break;
		default:
			char msg[50];
			snprintf(msg, sizeof(msg),"unknown (%d)", rotationState);
			answer["rotationState"] = msg;
	}
	answer["currentAngle"] = currentAngle;
	answer["requiredAngle"] = requiredAngle;
	answer["closestTrack"] = closestTrack;
	answer["closestDelta"] = closestDelta;
	char buffer[1024];
	serializeJsonPretty(answer, buffer, sizeof(buffer));
	request->send(200, "application/json", String(buffer));
}

// Called when a /pos/<x_percent>/<y_percent> click is received
void clickReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	// Try to locate closest track from clicked image
	int8_t closestIndex = -1;										// Init closest track index

	int separator1 = position.indexOf("/");							// Position of first "/"
	if (separator1 >= 0) {
		int separator2 = position.indexOf("/", separator1+1); 		// Position of second "/"
		if (separator2 > 0) {
			// Extract x and y values from request
			float x = position.substring(separator1+1, separator2).toFloat();
			float y = position.substring(separator2+1).toFloat();
            float closestDistance = 9999.0;
			// Loop through all used tracks
			for (uint8_t i = 1; i <= trackCount; i++) {
				// Compute track x/y percentage given angle
				float xPos = 50.0 + (radius * cos(indexes[i] * PI / 180.0));
				float yPos = 50.0 - (radius * sin(indexes[i] * PI / 180.0));
				// Test difference against max delta
				if (abs(x - xPos) < maxDelta and abs(y - yPos) < maxDelta) {
					// Compute distance
					float distance = sqrt(pow(abs(x - xPos), 2) + pow(abs(y - yPos), 2));
					// If distance is shorter than previous one, save index and distance
					if (distance < closestDistance) {
						closestDistance = distance;
						closestIndex = i;
					}
				}
			}
			if (traceCode) {
				printInfo("Index: %d\n", closestIndex);
			}
			// Turn table to closest index, if found
			if (closestIndex >=0) {
				moveToTrack(closestIndex);
			}
		}
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Called when /move/<micro step count> received
void moveReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");								// Position of first "/"
	if (separator1 >= 0) {
		// Extract micro step (signed) count and move
		int16_t count = position.substring(separator1+1).toInt();
		stepper.setDirection(count);
		microStepsToGo = abs(count);
		requiredAngle = -1;
		startRotation();
		char msg[50];											// Buffer for message
		snprintf(msg, sizeof(msg),"<status>Ok, moving %d micro steps</status>", count);
		request->send_P(200, "", msg);
	}
	char msg[50];											// Buffer for message
	snprintf(msg, sizeof(msg),"<status>Can't understand %s</status>", request->url().c_str());
	request->send_P(400, "", msg);
}

// Called when /imgref is received
void imgRefReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	// Set image offset (difference with current version)
	imageOffsetAngle = encoder.floatModuloPositive(imageOffsetAngle - currentAngle, 360.0);
	writeSettings();
	moveToTrack(1);			// As we moved image, we moved turntable. Move it back to zero.
	request->send_P(200, "", "<status>Ok</status>");
}

// Called when /goto/<track number> is received
void gotoReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position of first "/"
	if (separator1 >= 0) {
		// Extract track number to move to
		int8_t track = position.substring(separator1+1).toInt();
		// Check for track limits (as users can send this request)
		if (track > 0 && track <= trackCount)  {
			moveToTrack(track);										// Move to given track
			char msg[50];											// Buffer for message
			snprintf(msg, sizeof(msg),"<status>Ok, going to track %d</status>", track);
			request->send_P(200, "", msg);
			return;
		}
		char msg[50];											// Buffer for message
		snprintf(msg, sizeof(msg),"<status>Bad track number %d, should be 1-%d", track, trackCount);
		request->send_P(400, "", msg);
		return;
	}
	char msg[50];											// Buffer for message
	snprintf(msg, sizeof(msg),"<status>Can't understand %s</status>", request->url().c_str());
	request->send_P(400, "", msg);
}

// Called when /setangle/<track number> is received
void setAngleReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position of first "/"
	if (separator1 >= 0) {
		// Extract track number
		int8_t track = position.substring(separator1+1).toInt();
		if (track == 1) {											// Special case of first track
			indexes[1] = 0.0;										// Angle is zerp
			encoderOffsetAngle = encoder.floatModuloPositive(
					encoderOffsetAngle + currentAngle, 360.0);		// Set encoder offset
			writeSettings();
		} else {
			// Check for track limits (as users can send this request)
			if (track > 0 && track <= trackCount)  {
				indexes[track] = currentAngle;							// Set current angle to track
				writeSettings();
				request->send_P(200, "", "<status>Ok</status>");
				return;
			}
			char msg[50];											// Buffer for message
			snprintf(msg, sizeof(msg),"<status>Bad track number %d, should be 1-%d", track, trackCount);
			request->send_P(400, "", msg);
			return;
		}
	}
	char msg[50];											// Buffer for message
	snprintf(msg, sizeof(msg),"<status>Can't understand %s</status>", request->url().c_str());
	request->send_P(400, "", msg);
}

// Called when /changed/<variable name>/<variable value> is received
void setChangedReceived(AsyncWebServerRequest *request) {
	String position = request->url().substring(1);
	if (traceCode) {
		printInfo("Received %s\n", position.c_str());
	}
	int separator1 = position.indexOf("/");							// Position of first "/"
	if (separator1 >= 0) {
		int separator2 = position.indexOf("/", separator1+1); 		// Position of second "/"
		if (separator2 > 0) {
			// Extract field name and value
			String fieldName = position.substring(separator1+1, separator2);
			String fieldValue = position.substring(separator2+1);
			fieldValue.toLowerCase();
			// Check against known field names and set value accordingly
			if (fieldName == "trackCount") {
				trackCount = fieldValue.toInt();
			} else if (fieldName == "degreesPerStep") {
				degreesPerStep = fieldValue.toFloat();
			} else if (fieldName == "microStepsPerStep") {
				microStepsPerStep = fieldValue.toInt();
			} else if (fieldName == "invertStepper") {
				invertStepper = (fieldValue == "true");
			} else if (fieldName == "stepperReduction") {
				stepperReduction = fieldValue.toFloat();
			} else if (fieldName == "clockwiseIncrement") {
				clockwiseIncrement = (fieldValue == "true");
			} else if (fieldName == "driverMinimalMicroSec") {
				driverMinimalMicroSec = fieldValue.toInt();
			} else if (fieldName == "requiredRPM") {
				requiredRPM = fieldValue.toFloat();
			} else if (fieldName == "slaveId") {
				slaveId = fieldValue.toInt();
			} else if (fieldName == "regId") {
				regId = fieldValue.toInt();
			} else if (fieldName == "radius") {
				radius = fieldValue.toFloat();
			} else if (fieldName == "maxDelta") {
				maxDelta = fieldValue.toInt();
			} else if (fieldName == "traceDebug") {
				traceDebug = (fieldValue == "true");
			} else if (fieldName == "traceCode") {
				traceCode = (fieldValue == "true");
			} else if (fieldName == "traceJava") {
				traceJava = (fieldValue == "true");
			} else if (fieldName == "ssid") {
				ssid = fieldValue;
			} else if (fieldName == "pwd") {
				pwd = fieldValue;
			} else if (fieldName == "name") {
				espName = fieldValue;
			} else if (fieldName == "enableEncoder") {
				enableEncoder = (fieldValue == "true");
			} else if (fieldName == "adjustPosition") {
				adjustPosition = (fieldValue == "true");
			} else if (fieldName == "enableSound") {
				enableSound = (fieldValue == "true");
			} else if (fieldName == "soundVolume") {
				soundVolume = fieldValue.toInt();
			} else if (fieldName == "beforeSoundIndex") {
				beforeSoundIndex = fieldValue.toInt();
			} else if (fieldName == "beforeSoundDuration") {
				beforeSoundDuration = fieldValue.toFloat();
			} else if (fieldName == "moveSoundIndex") {
				moveSoundIndex = fieldValue.toInt();
			} else if (fieldName == "afterSoundIndex") {
				afterSoundIndex = fieldValue.toInt();
			} else if (fieldName == "afterSoundDuration") {
				afterSoundDuration = fieldValue.toFloat();
			} else {
				// This is not a known field
				printError("Can't set field %s\n", fieldName.c_str());
				char msg[50];											// Buffer for message
				snprintf(msg, sizeof(msg),"<status>Bad field name %s", fieldName.c_str());
				request->send_P(400, "", msg);
				return;
			}
			writeSettings();
		}
	}
	request->send_P(200, "", "<status>Ok</status>");
}

// Sends a 404 error with requested file name
void send404Error(AsyncWebServerRequest *request) {
	char msg[50];
	snprintf_P(msg, sizeof(msg), PSTR("File %s not found"), request->url().c_str());
	request->send(404, "text/plain", msg);
	if (traceCode) {
		Serial.println(msg);
	}
}

// Called when a request can't be mapped to existing ones
void notFound(AsyncWebServerRequest *request) {
	send404Error(request);
}

//	---- OTA routines ----

// Called when OTTA starts
void onStartOTA() {
	if (ArduinoOTA.getCommand() == U_FLASH) {						// Program update
		if (traceCode) {
			Serial.println("Starting firmware update");
		}
	} else { 														// File system uodate
		if (traceCode) {
			Serial.println("Starting file system update");
		}
		LittleFS.end();
	}
}
		
// Called when OTA ends
void onEndOTA() {
	if (traceCode) {
		Serial.println("End of update");
	}
}
	

// Called when OTA error occurs
void onErrorOTA(ota_error_t erreur) {
	Serial.print("OTA error(");
	Serial.print(erreur);
	Serial.print(") : Error ");
	if (erreur == OTA_AUTH_ERROR) {
		Serial.println("authentication");
	} else if (erreur == OTA_BEGIN_ERROR) {
		Serial.println("starting");
	} else if (erreur == OTA_CONNECT_ERROR) {
		Serial.println("connecting");
	} else if (erreur == OTA_RECEIVE_ERROR) {
		Serial.println("receiving");
	} else if (erreur == OTA_END_ERROR) {
		Serial.println("terminating");
	} else {
		Serial.println("unknown !");
	}
}

//	---- Start of program initialization code

void setup() {
	Serial.begin(74880);											// Init serial port to 74880 bds (~ 7.500 characters per second)
	Serial.println("Initializing...");

	// Starts flash file system
	LittleFS.begin();

	// Load preferences
	if (!readSettings()) {
		Serial.printf("No settings, stopping !\n");
		while (true) {
			yield();
		}
	};
	
	WiFi.hostname(espName.c_str());									// Defines this module name
	IPAddress adresseIP;											// Client or server IP address

	ssid.trim();
	if (ssid != "") {												// If SSID is given, try to connect to
		Serial.printf("\nConnect to %s ", ssid.c_str());

		uint16_t loopCount = 0;										// Tentative cound
		WiFi.begin(ssid.c_str(), pwd.c_str());						// Start to connect to existing SSID
		while (WiFi.status() != WL_CONNECTED && loopCount < 120) {	// Wait for connection or 120 loops (1 mn)
			delay(500);												// Wait for 0.5 s
			Serial.print(".");										// Display a dot
			loopCount++;
		}															// Loop
		adresseIP = WiFi.localIP();									// Client IP address
	}

	if (WiFi.status() != WL_CONNECTED) {							// If we're not connected
		char buffer[32];
		// Load this Wifi access point name as "PontTournant" plus ESP chip Id
		snprintf(buffer, sizeof(buffer),"PontTournant_%X", ESP.getChipId());
		Serial.printf("\nCreating %s access point\n", buffer);
		WiFi.softAP(buffer, "");									// Starts Wifi access point
		adresseIP = WiFi.softAPIP();								// Server IP address
	}

	Serial.println();
	Serial.print("Connect to http://");
	Serial.print(adresseIP);										// Display client or server IP address
	Serial.print("/ or http://");
	Serial.print(espName);
	Serial.println("/");

	// OTA trace
	ArduinoOTA.onStart(onStartOTA);
	ArduinoOTA.onEnd(onEndOTA);
	ArduinoOTA.onError(onErrorOTA);

	//ArduinoOTA.setPassword("my OTA password");					// Uncomment to set an OTA password
	ArduinoOTA.begin();												// Initialize OTA

	events.onConnect([](AsyncEventSourceClient *client){			// Routine called when a client connects
		float angle = encoder.computeAngle(resultValue);
		// Sends an "angle" events to connecting client
		char msg[10];
		if (enableEncoder) {
			snprintf_P(msg, sizeof(msg), PSTR("%.2f"), angle);
		} else {
			snprintf_P(msg, sizeof(msg), PSTR("%.2f"), currentAngle);
		}
		client->send(msg, "angle");
	});

    // List of URL to be intercepted and treated locally before a standard treatment
	//	These URL can be used as API
    webServer.on("/status", HTTP_GET, statusReceived);				// /status request
    webServer.on("/goto", HTTP_GET, gotoReceived);					// /goto request
	// These URL can be used in browser (in addition to /edit, managed in LittleFsEditor)
    webServer.on("/setup", HTTP_GET, setupReceived);				// /setup request
    webServer.on("/settings", HTTP_GET, settingsReceived);			// /settings request
    webServer.on("/debug", HTTP_GET, debugReceived);				// /debug request
	// These URL are used internally by setup.htm - Use them at your own risk!
    webServer.on("/pos", HTTP_GET, clickReceived);					// /pos request
    webServer.on("/move", HTTP_GET, moveReceived);					// /move request
    webServer.on("/imgref", HTTP_GET, imgRefReceived);				// /imgref request
    webServer.on("/changed", HTTP_GET, setChangedReceived);			// /changed request
    webServer.on("/setangle", HTTP_GET, setAngleReceived);			// /setangle request
	webServer.addHandler(&events);									// Define web events
	webServer.addHandler(new LittleFSEditor());						// Define file system editor
	webServer.onNotFound (notFound);								// To be called when URL is not known
	webServer.serveStatic("/",LittleFS, "/").setDefaultFile("index.htm"); // Serve "/", default page = index.htm
	webServer.begin();												// Start Web server

	stepper.begin();												// Stepper init

	digitalWrite(runPin, LOW);										// Set rotation pin to low
	pinMode(runPin, OUTPUT);										// Set rotation pin to output mode
	

	encoder.setCallback(modbusCb);									// Modbus end request callback
	encoder.begin();												// Encoder init

	player.set_volume(soundVolume);									// Define volume (max = 30)
	player.set_play_mode(1);										// Play mode: loop on file
}

// ---- Permanent loop ----
void loop() {
	// Manage stepper rotation
	if (rotationState == stepperRunning) {							// Are we rotating?
		stepper.turnOneMicroStep();									// Turn one micro step
		currentAngle = encoder.floatModuloPositive(					// Make result in range 0-360
			currentAngle + stepper.anglePerMicroStep()				// Update current angle
			, 360.0);
		microStepsToGo--;											// Remove one micro step
		if (!microStepsToGo) {										// Do we end rotation?
			stopRotation();											// Stop rotation
		}
	} else if (rotationState == stepperStarting) {					// Are we starting?
		if ((millis() - playerStartedTime) > playerDuration) {		// Do we exhaust playing time?
			if (moveSoundIndex) {									// Do we have a rotation sound?
				playIndex(moveSoundIndex);							// Play rotation sound
			} else {
				playStop();											// Stop starting sound
			}
			rotationState = stepperRunning;							// We're now running
		}
	} else if (rotationState == stepperStopping) {					// Are we stopping?
		if ((millis() - playerStartedTime) > playerDuration) {		// Do we exhaust playing time?
			playStop();												// Stop stopping sound
			rotationState = stepperStopped;							// Set state to stopped
			digitalWrite(runPin, LOW);								// Set rotation pin to low
		}
	} else if (rotationState == stepperFixing) {					// Are we fixing?
		stepper.turnOneMicroStep();									// Turn one micro step
		currentAngle = encoder.floatModuloPositive(					// Make result in range 0-360
			currentAngle + stepper.anglePerMicroStep()				// Update current angle
			, 360.0);
		microStepsToGo--;											// Remove one micro step
		if (!microStepsToGo) {										// Do we end rotation?
			rotationState = stepperStopped;							// Set state back to stopper
		}
	} else if (rotationState == stepperStopped) {					// Are we stopped?
		if (!enableEncoder) {										// Is encoder disabled (manual mode)?
			fixPosition();											// Fix position
		}
	}

	// Manage encoder
	if (enableEncoder) {
		if (encoder.active()){										// Is encoder active?
			encoder.loop();											// Give habd to encoder
		}
	}
	
			if (enableEncoder) {									// Encoder enabled?
		if ((millis() - encoderReadTime) > 1000) {					// Last reading more than a second?
					encoderReading = true;							// Encoder reading
					encoderReadTime = millis();						// Read start time
					encoder.getAngle(&resultValue);					// Restart request
					}
				}

	if ((millis() - resultValueTime) > 500) {						// Last result sent more than 0.5 seconds?
				if (abs(currentAngle - lastAngleSent) >= MIN_DELTA_ANGLE) {	// Send message only if there's a slight difference
			if (traceCode) {
				printInfo("Angle: %f\n", currentAngle);
					}
					// Sends an "angle" event to browser
					char msg[10];									// Message buffer
					snprintf_P(msg, sizeof(msg), PSTR("%.2f"), currentAngle);	// Convert angle to message
					events.send(msg, "angle");						// Send an "angle" message
					lastAngleSent = currentAngle;					// Save last angle sent
				}
				resultValueTime = millis();							// Save time of last result
	}

	ArduinoOTA.handle();											// Give hand to OTA
}