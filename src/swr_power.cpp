#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"


PowerPointBounds determineBounds(uint16_t adcValue, boolean dummy) {
  etl::iset<float, std::less<float>>::const_iterator itr;
  etl::iset<float, std::less<float>>::const_iterator itrEnd;

  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPointsDummy = calibrationPowerPointsDummy();
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointsOpen = calibrationPowerPointsOpen();
  if( dummy ) {
    itr = powerPointsDummy.begin();
    itrEnd = powerPointsDummy.end();
  }
  else {
    itr = powerPointsOpen.begin();
    itrEnd = powerPointsOpen.end();
  }


  PowerPointBounds bounds = {-1.0, -1.0, 0, 0, false };

  //check if we are above the bounds.
  float highestPower =  highestPowerPoint(dummy);
  CalibrationData highestCalibrationData = calibrationData(highestPower, dummy);

  //we are above the bounds of the calibration
  if( adcValue > highestCalibrationData.fwd )
    bounds.outOfBounds = true;

  while (itr != itrEnd)
  {
    float currentPowerPoint = *itr++;
    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, dummy);
    uint16_t currentAdcValue = (dummy ? currentCalibrationData.fwd : currentCalibrationData.rvr);
    if( !bounds.outOfBounds ) {
      if( (currentAdcValue > bounds.lowAdcValue || bounds.lowVoltagePoint == -1.0) &&  currentAdcValue < adcValue ) {
        bounds.lowAdcValue = currentAdcValue;
        bounds.lowVoltagePoint = powerToVoltage(currentPowerPoint);
      }
      else if( (currentAdcValue < bounds.highAdcValue || bounds.highVoltagePoint == -1.0) &&  currentAdcValue >= adcValue ) {
        bounds.highAdcValue = currentAdcValue;
        bounds.highVoltagePoint = powerToVoltage(currentPowerPoint);
      }
    }
    else {
      if( currentAdcValue > bounds.highAdcValue || bounds.highVoltagePoint == -1.0) {
        bounds.lowAdcValue = bounds.highAdcValue;
        bounds.lowVoltagePoint = bounds.highVoltagePoint;
        bounds.highAdcValue = currentAdcValue;
        bounds.highVoltagePoint = powerToVoltage(currentPowerPoint);
      }
    }
  }

  if( bounds.lowVoltagePoint == -1.0 ) {
    if( dummy ) {
      bounds.lowAdcValue = 0;
      bounds.lowVoltagePoint = 0;
    }
    else {
      float lowestPower = lowestPowerPoint(true);
      CalibrationData lowestCalibrationData = calibrationData(lowestPower, true);
      bounds.lowAdcValue = lowestCalibrationData.rvr;
      bounds.lowVoltagePoint = 0.0;
    }
  }

  return bounds;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float adcToVoltage(uint16_t adcValue, boolean forward) {
  PowerPointBounds bounds = determineBounds(adcValue, forward);
  return mapFloat(adcValue, bounds.lowAdcValue, bounds.highAdcValue, bounds.lowVoltagePoint, bounds.highVoltagePoint);
}

uint16_t voltageToAdc(float voltage, boolean forward) {
  PowerPointBounds bounds = determineBounds(voltage, forward);
  return mapFloat(voltage, bounds.lowVoltagePoint, bounds.highVoltagePoint, bounds.lowAdcValue, bounds.highAdcValue);
}

float voltageToPower(float voltage) {
  return (voltage*voltage) / CHARACTERISTIC_IMPEDANCE;
}

float powerToVoltage(float power) {
  return sqrt(power * CHARACTERISTIC_IMPEDANCE);
}

void updateComplex(float *magnitudeDb, float *phase) {
  float adcMax = analogRead(COMPLEX_VREF_PIN);

  float adcMagnitude = analogRead(COMPLEX_MAGNITUDE_PIN);
  float magnitudeNormalized = (adcMagnitude / adcMax) * 2.0 - 1.0;
  *magnitudeDb = magnitudeNormalized * 30.0;

  float adcPhase = analogRead(COMPLEX_PHASE_PIN);
  float phaseNormalized = 1.0 - (adcPhase / adcMax);
  *phase = phaseNormalized * 180.0;
}

void updatePower(float *power_fwd, float *power_rvr) {
  uint16_t adcFwdValue = analogRead(POWER_FWD_PIN);
  float voltageFwd = adcToVoltage(adcFwdValue, true);
  *power_fwd = voltageToPower(voltageFwd);

  uint16_t adcRvrValue = analogRead(POWER_RVR_PIN);
  float voltageRvr = adcToVoltage(adcRvrValue, false);
  *power_rvr = voltageToPower(voltageRvr);
}

float lowestPowerPoint(boolean dummy) {
  etl::iset<float, std::less<float>>::const_iterator itr;
  etl::iset<float, std::less<float>>::const_iterator itrEnd;

  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPointsDummy = calibrationPowerPointsDummy();
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointsOpen = calibrationPowerPointsOpen();
  if( dummy ) {
    itr = powerPointsDummy.begin();
    itrEnd = powerPointsDummy.end();
  }
  else {
    itr = powerPointsOpen.begin();
    itrEnd = powerPointsOpen.end();
  }

  float power = -1.0;
  while (itr != itrEnd)
  {
    float powerPoint = *itr++;
    if(powerPoint < power || power == -1.0)
      power = powerPoint;
  }
  return power;
}

float highestPowerPoint(boolean dummy) {
  etl::iset<float, std::less<float>>::const_iterator itr;
  etl::iset<float, std::less<float>>::const_iterator itrEnd;

  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPointsDummy = calibrationPowerPointsDummy();
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointsOpen = calibrationPowerPointsOpen();
  if( dummy ) {
    itr = powerPointsDummy.begin();
    itrEnd = powerPointsDummy.end();
  }
  else {
    itr = powerPointsOpen.begin();
    itrEnd = powerPointsOpen.end();
  }

  float power = -1.0;
  while (itr != itrEnd)
  {
    float powerPoint = *itr++;
    if(powerPoint > power)
      power = powerPoint;
  }
  return power;
}
