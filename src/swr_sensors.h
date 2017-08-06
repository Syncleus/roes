#ifndef _SWR_SENSORS_H_
#define _SWR_SENSORS_H_

#include <Arduino.h>

struct SensorData {
  float fwdPower;
  float reflPower;
  float differentialMagnitudeDb;
  float differentialPhaseDeg;
  float differentialMagnitudeDbShifted;
  float differentialPhaseDegShifted;
};

struct RawSensorData {
  uint16_t fwdVoltage;
  uint16_t reflVoltage;
  uint16_t differentialMagnitude;
  uint16_t differentialVref;
  uint16_t differentialPhase;
  uint16_t differentialMagnitudeShifted;
  uint16_t differentialVrefShifted;
  uint16_t differentialPhaseShifted;
};

SensorData readSensors(SensorData lastData);
RawSensorData readSensorsRaw();

#endif /* _SWR_SENSORS_H_ */
