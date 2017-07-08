#include "swr_power.h"
#include "swr_constants.h"
#include "swr_eeprom.h"

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
