#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"

float calibrate_fwd_slope = 0.0;
float calibrate_fwd_intercept = 0.0;
float calibrate_rvr_slope = 0.0;
float calibrate_rvr_intercept = 0.0;
float calibrate_ratio_slope = 0.0;
float calibrate_ratio_intercept = 0.0;

void powerSetup() {
  calibrateFwd(LOW_POWER, calibrationLowFwd(), HIGH_POWER, calibrationHighFwd());
  calibrateRvr(LOW_POWER, calibrationLowRvr(), HIGH_POWER, calibrationHighRvr());
  calibrateRatio(LOW_POWER, calibrationLowRatio(), HIGH_POWER, calibrationHighRatio());
}

float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc) {
  return (highVoltage - lowVoltage) / ((float) (highAdc - lowAdc));
}

float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc) {
  return voltage - slope * ((float)adc);
}

float calculateCalibrationRatioSlope(float lowPower, float lowValue, float highPower, float highValue) {
  return ((float) (highValue - lowValue)) / (highPower - lowPower);
}

float calculateCalibrationRatioIntercept(float slope, float power, float value) {
  return ((float)value) - slope * power;
}

void calibrateRatio(float lowPower, float lowRatio, float highPower, float highRatio) {
  calibrate_ratio_slope = calculateCalibrationRatioSlope(lowPower, lowRatio, highPower, highRatio);
  calibrate_ratio_intercept = calculateCalibrationRatioIntercept(calibrate_ratio_slope, highPower, highRatio);
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


void updatePower(float &power_fwd, float &power_rvr) {
  uint16_t adcFwdValue = analogRead(POWER_FWD_PIN);
  power_fwd = calculateFwdPower(adcFwdValue);

  float reverseRatio = power_fwd * calibrate_ratio_slope + calibrate_ratio_intercept;

  float reverseBaseline = ((float)adcFwdValue) / reverseRatio;
  int32_t adcRvrValue = analogRead(POWER_RVR_PIN);
  int32_t adjustedAdcRvrValue = ((float)adcRvrValue) - reverseBaseline;
  power_rvr = calculateRvrPower(adjustedAdcRvrValue >= 0 ? adjustedAdcRvrValue : 0);
}
