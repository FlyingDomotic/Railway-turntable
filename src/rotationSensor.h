#ifndef RotationSensor_h
	#define RotationSensor_h

	#include <inttypes.h>
	#include <ModbusRTU.h>
	#include <SoftwareSerial.h>

	#ifdef __cplusplus
		class RotationSensor {
			public:
				RotationSensor(uint8_t _rxPin,  uint8_t _txPin, bool _traceDebug);
				void begin(void);
				void setParams(float _offsetAngle, bool _traceDebug);
				void loop(void);
				bool active(void);
				float floatModuloPositive(float f, float modulo);
				RotationSensor& setCallback(std::function<Modbus::ResultCode(Modbus::ResultCode event, uint16_t transactionId, void* data)> callback);
				void getAngle(uint16_t *result);
				float computeAngle(uint16_t valueRead);

			private:
				std::function<Modbus::ResultCode(Modbus::ResultCode event, uint16_t transactionId, void* data)> callback;
				float offsetAngle;
				uint8_t rxPin;
				uint8_t txPin;
				bool traceDebug;
				SoftwareSerial modbusSerial;
				ModbusRTU modbusRtu;
		};
	#endif
#endif
