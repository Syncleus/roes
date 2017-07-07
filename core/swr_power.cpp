#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"

float calibrate_fwd_slope = 0.0;
float calibrate_fwd_intercept = 0.0;
float calibrate_rvr_slope = 0.0;
float calibrate_rvr_intercept = 0.0;

void powerSetup() {
  calibrateFwd(LOW_POWER, calibrationLowFwd(), HIGH_POWER, calibrationHighFwd());
  calibrateRvr(LOW_POWER, calibrationLowRvr(), HIGH_POWER, calibrationHighRvr());
}

float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc) {
  return (highVoltage - lowVoltage) / ((float) (highAdc - lowAdc));
}

float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc) {
  return voltage - slope * ((float)adc);
}

void calibrateFwd(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
  float lowVoltage = powerToVoltage(lowPower);
  float highVoltage = powerToVoltage(highPower);
  calibrate_fwd_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
  calibrate_fwd_intercept = calculateCalibrationIntercept(calibrate_fwd_slope, highVoltage, highAdc);
}

void calibrateRvr(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
  float lowVoltage = powerToVoltage(lowPower);
  float highVoltage = powerToVoltage(highPower);
  calibrate_rvr_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
  calibrate_rvr_intercept = calculateCalibrationIntercept(calibrate_rvr_slope, highVoltage, highAdc);
}

float calculateFwdPower(uint16_t adcValue) {
  float calculatedVoltage = ((float)adcValue) * calibrate_fwd_slope + calibrate_fwd_intercept;
  return voltageToPower(calculatedVoltage);
}

float calculateRvrPower(uint16_t adcValue) {
  float calculatedVoltage = ((float)adcValue) * calibrate_rvr_slope + calibrate_rvr_intercept;
  return voltageToPower(calculatedVoltage);
}

float voltageToPower(float voltage) {
  return (voltage*voltage) / 50.0;
}

float powerToVoltage(float power) {
  return sqrt(power * 50.0);
}

float readPowerFwd() {
  uint16_t adcValue = analogRead(POWER_FWD_PIN);
  return calculateFwdPower(adcValue);
}

float readPowerRvr() {
  uint16_t adcValue = analogRead(POWER_RVR_PIN);
  return calculateRvrPower(adcValue);
}

void updatePower(float &power_fwd, float &power_rvr) {
  power_fwd = readPowerFwd();
  power_rvr = readPowerRvr();
}
