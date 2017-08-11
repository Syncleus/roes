#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"

#include <complex.h>


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
    uint16_t currentAdcValue = (dummy ? currentCalibrationData.fwd : currentCalibrationData.refl);
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
      bounds.lowAdcValue = lowestCalibrationData.refl;
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

float dbToSwr(float magnitudeDb) {
  float linearFactor = pow(10.0, -1.0 * magnitudeDb / 20.0);

  if ( linearFactor <= 0 )
    return 1.0;

  return (linearFactor + 1.0) / (linearFactor - 1.0);
}

Complex polarToComplex(float magnitude, float phase) {
  double real = magnitude * cos(phase * PI / 180.0);
  double imaginary = magnitude * sin(phase * PI / 180.0);
  return Complex(real, imaginary);
}

float powerToSwr(float power_fwd, float power_rvr) {
  if ( power_rvr <= 0 )
    return 1.0;
  float pwrs = sqrt(power_rvr / power_fwd);
  return (1.0 + pwrs) / (1.0 - pwrs);
}

float logBased(float value, float base) {
  return log10(value) / log10(base);
}

float powerToDbm(float power) {
  return 10.0 * log10(power) + 30.0;
}

Complex complexLoadFromReflection(float magnitude, float phase) {
  Complex refl = polarToComplex(magnitude, phase);
  Complex characteristicZ(CHARACTERISTIC_IMPEDANCE, 0.0);
  Complex loadZ = (characteristicZ * (one + refl)) / (one - refl);
  return loadZ;
}

float calibratedPhase(float phase, float power) {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointsOpen = calibrationPowerPointsOpen();

  etl::iset<float, std::less<float>>::const_iterator itr = powerPointsOpen.begin();
  etl::iset<float, std::less<float>>::const_iterator itrEnd = powerPointsOpen.end();

  float closestPowerPoint = -1.0;
  while (itr != itrEnd)
  {
    float currentPowerPoint = *itr++;
    if( closestPowerPoint < 0.0 )
      closestPowerPoint = currentPowerPoint;
    else {
      if( abs(power - currentPowerPoint) < abs(power - closestPowerPoint) )
        closestPowerPoint = currentPowerPoint;
    }
  }

  CalibrationData calData = calibrationData(closestPowerPoint, false);

  float calPhase = (1.0 - (((float)calData.phase) / ((float)calData.vref))) * 180.0;

  return phase - calPhase;
}

float expectedPhaseShift(float power) {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointsOpen = calibrationPowerPointsOpen();

  etl::iset<float, std::less<float>>::const_iterator itr = powerPointsOpen.begin();
  etl::iset<float, std::less<float>>::const_iterator itrEnd = powerPointsOpen.end();

  float closestPowerPoint = -1.0;
  while (itr != itrEnd)
  {
    float currentPowerPoint = *itr++;
    if( closestPowerPoint < 0.0 )
      closestPowerPoint = currentPowerPoint;
    else {
      if( abs(power - currentPowerPoint) < abs(power - closestPowerPoint) )
        closestPowerPoint = currentPowerPoint;
    }
  }

  CalibrationData calData = calibrationData(closestPowerPoint, false);
  float calPhase = (1.0 - (((float)calData.phase) / ((float)calData.vref))) * 180.0;
  float calPhaseShifted = (1.0 - (((float)calData.phaseShifted) / ((float)calData.vrefShifted))) * 180.0;

  return calPhaseShifted - calPhase;
}
