#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"

//float calibrate_fwd_slope = 0.0;
//float calibrate_fwd_intercept = 0.0;
//float calibrate_rvr_slope = 0.0;
//float calibrate_rvr_intercept = 0.0;
//float calibrate_ratio_slope = 0.0;
//float calibrate_ratio_intercept = 0.0;

//void powerSetup() {
//  calibrateFwd(LOW_POWER, calibrationLowFwd(), HIGH_POWER, calibrationHighFwd());
//  calibrateRvr(LOW_POWER, calibrationLowRvr(), HIGH_POWER, calibrationHighRvr());
//  calibrateRatio(LOW_POWER, calibrationLowRatio(), HIGH_POWER, calibrationHighRatio()); 
//}

//float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc) {
//  return (highVoltage - lowVoltage) / ((float) (highAdc - lowAdc));
//}

//float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc) {
//  return voltage - slope * ((float)adc);
//}
//
//float calculateCalibrationRatioSlope(float lowPower, float lowValue, float highPower, float highValue) {
//  return ((float) (highValue - lowValue)) / (highPower - lowPower);
//}
//
//float calculateCalibrationRatioIntercept(float slope, float power, float value) {
//  return ((float)value) - slope * power;
//}

//void calibrateRatio(float lowPower, float lowRatio, float highPower, float highRatio) {
//  calibrate_ratio_slope = calculateCalibrationRatioSlope(lowPower, lowRatio, highPower, highRatio);
//  calibrate_ratio_intercept = calculateCalibrationRatioIntercept(calibrate_ratio_slope, highPower, highRatio);
//}
//
//void calibrateFwd(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
//  float lowVoltage = powerToVoltage(lowPower);
//  float highVoltage = powerToVoltage(highPower);
//  calibrate_fwd_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
//  calibrate_fwd_intercept = calculateCalibrationIntercept(calibrate_fwd_slope, highVoltage, highAdc);
//}
//
//void calibrateRvr(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
//  float lowVoltage = powerToVoltage(lowPower);
//  float highVoltage = powerToVoltage(highPower);
//  calibrate_rvr_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
//  calibrate_rvr_intercept = calculateCalibrationIntercept(calibrate_rvr_slope, highVoltage, highAdc);
//}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float adcToFwdVoltage(uint16_t adcValue) {
  return mapFloat(adcValue, calibrationLowFwd(), calibrationHighFwd(), powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER));
}

uint16_t fwdVoltageToAdc(float voltage) {
  return mapFloat(voltage, powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER), calibrationLowFwd(), calibrationHighFwd());
}

float adcToRvrVoltage(uint16_t adcValue) {
  return mapFloat(adcValue, calibrationLowRvr(), calibrationHighRvr(), powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER));
}

uint16_t RvrVoltageToAdc(float voltage) {
  return mapFloat(voltage, powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER), calibrationLowRvr(), calibrationHighRvr());
}

float fwdVoltageToRatio(float voltage) {
  return mapFloat(voltage, powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER), calibrationLowRatio(), calibrationHighRatio());
}

float ratioToFwdVoltage(float ratio) {
  return mapFloat(ratio, calibrationLowRatio(), calibrationHighRatio(), powerToVoltage(LOW_POWER), powerToVoltage(HIGH_POWER));
}

float voltageToPower(float voltage) {
  return (voltage*voltage) / 50.0;
}

float powerToVoltage(float power) {
  return sqrt(power * 50.0);
}

void updatePower(float &power_fwd, float &power_rvr) {
  uint16_t adcFwdValue = analogRead(POWER_FWD_PIN);
  float voltageFwd = adcToFwdVoltage(adcFwdValue);
  power_fwd = voltageToPower(voltageFwd);

  float reverseRatio = fwdVoltageToRatio(voltageFwd);

  float reverseBaseline = ((float)adcFwdValue) / reverseRatio;
  int32_t adcRvrValue = analogRead(POWER_RVR_PIN);
  int32_t adjustedAdcRvrValue = adcRvrValue - reverseBaseline;
  uint16_t boundedAdcRvrValue = (adjustedAdcRvrValue >= 0 ? adjustedAdcRvrValue : 0);
  float rvrVoltage = adcToRvrVoltage(boundedAdcRvrValue);
  power_rvr = voltageToPower(rvrVoltage);
}
