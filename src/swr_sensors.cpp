#include "swr_sensors.h"
#include "swr_constants.h"
#include "swr_power.h"

SensorData readSensors() {
  RawSensorData rawData = readSensorsRaw();

  SensorData data;

  float voltageFwd = adcToVoltage(rawData.fwdVoltage, true);
  data.fwdVoltage = voltageToPower(voltageFwd);

  float voltageRvr = adcToVoltage(rawData.reflVoltage, false);
  data.reflVoltage = voltageToPower(voltageRvr);

  float magnitudeNormalized = (rawData.differentialMagnitude / rawData.differentialVref) * 2.0 - 1.0;
  data.differentialMagnitudeDb = magnitudeNormalized * 30.0;

  float phaseNormalized = 1.0 - (rawData.differentialPhase / rawData.differentialVref);
  data.differentialPhaseDeg = phaseNormalized * 180.0;

  float magnitudeNormalizedShifted = (rawData.differentialMagnitudeShifted / rawData.differentialVrefShifted) * 2.0 - 1.0;
  data.differentialMagnitudeDbShifted = magnitudeNormalizedShifted * 30.0;

  float phaseNormalizedShifted = 1.0 - (rawData.differentialPhaseShifted / rawData.differentialVrefShifted);
  data.differentialPhaseDegShifted = phaseNormalizedShifted * 180.0;

  return data;
}

RawSensorData readSensorsRaw() {
  RawSensorData data;

  data.fwdVoltage = analogRead(POWER_FWD_PIN);
  data.reflVoltage = analogRead(POWER_REFL_PIN);
  data.differentialVref = analogRead(DIFFERENTIAL_VREF_PIN);
  data.differentialPhase = analogRead(DIFFERENTIAL_PHASE_PIN);
  data.differentialMagnitude = analogRead(DIFFERENTIAL_MAGNITUDE_PIN);
  data.differentialVrefShifted = analogRead(SHIFTED_DIFFERENTIAL_VREF_PIN);
  data.differentialPhaseShifted = analogRead(SHIFTED_DIFFERENTIAL_PHASE_PIN);
  data.differentialMagnitudeShifted = analogRead(SHIFTED_DIFFERENTIAL_MAGNITUDE_PIN);

  return data;
}
