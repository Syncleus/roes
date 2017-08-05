#include "swr_sensors.h"
#include "swr_constants.h"
#include "swr_power.h"

SensorData readSensors(SensorData lastData) {
  RawSensorData rawData = readSensorsRaw();

  SensorData data;

  float voltageFwd = adcToVoltage(rawData.fwdVoltage, true);
  data.fwdVoltage = voltageToPower(voltageFwd);
  data.fwdVoltage = data.fwdVoltage * SENSOR_AVERAGING_ALPHA + lastData.fwdVoltage * (1.0 - SENSOR_AVERAGING_ALPHA);

  float voltageRvr = adcToVoltage(rawData.reflVoltage, false);
  data.reflVoltage = voltageToPower(voltageRvr);
  data.reflVoltage = data.reflVoltage * SENSOR_AVERAGING_ALPHA + lastData.reflVoltage * (1.0 - SENSOR_AVERAGING_ALPHA);

  float magnitudeNormalized = (((float)rawData.differentialMagnitude) / ((float)rawData.differentialVref)) * 2.0 - 1.0;
  data.differentialMagnitudeDb = magnitudeNormalized * 30.0;
  data.differentialMagnitudeDb = data.differentialMagnitudeDb * SENSOR_AVERAGING_ALPHA + lastData.differentialMagnitudeDb * (1.0 - SENSOR_AVERAGING_ALPHA);

  float phaseNormalized = 1.0 - (((float)rawData.differentialPhase) / ((float)rawData.differentialVref));
  data.differentialPhaseDeg = phaseNormalized * 180.0;
  data.differentialPhaseDeg = data.differentialPhaseDeg * SENSOR_AVERAGING_ALPHA + lastData.differentialPhaseDeg * (1.0 - SENSOR_AVERAGING_ALPHA);

  float magnitudeNormalizedShifted = (((float)rawData.differentialMagnitudeShifted) / ((float)rawData.differentialVrefShifted)) * 2.0 - 1.0;
  data.differentialMagnitudeDbShifted = magnitudeNormalizedShifted * 30.0;
  data.differentialMagnitudeDbShifted = data.differentialMagnitudeDbShifted * SENSOR_AVERAGING_ALPHA + lastData.differentialMagnitudeDbShifted * (1.0 - SENSOR_AVERAGING_ALPHA);

  float phaseNormalizedShifted = 1.0 - (((float)rawData.differentialPhaseShifted) / ((float)rawData.differentialVrefShifted));
  data.differentialPhaseDegShifted = phaseNormalizedShifted * 180.0;
  data.differentialPhaseDegShifted = data.differentialPhaseDegShifted * SENSOR_AVERAGING_ALPHA + lastData.differentialPhaseDegShifted * (1.0 - SENSOR_AVERAGING_ALPHA);

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
