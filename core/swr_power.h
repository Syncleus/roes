#ifndef _SWR_POWER_H_
#define _SWR_POWER_H_

#include <Arduino.h>

//void powerSetup();
//float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc);
//float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc);
//void calibrateFwd(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc);
//void calibrateRvr(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc);
//float calculateFwdVoltage(uint16_t adcValue);
//float calculateRvrVoltage(uint16_t adcValue);
//float voltageToPower(float voltage);
//float powerToVoltage(float power);
float adcToFwdVoltage(uint16_t adcValue);
uint16_t fwdVoltageToAdc(float voltage);
float adcToRvrVoltage(uint16_t adcValue);
uint16_t RvrVoltageToAdc(float voltage);
float fwdVoltageToRatio(float voltage);
float ratioToFwdVoltage(float ratio);
float voltageToPower(float voltage);
float powerToVoltage(float power);
void updatePower(float &power_fwd, float &power_rvr);
//float calculateCalibrationSlope(float lowVoltage, float lowValue, float highVoltage, float highValue);
//float calculateCalibrationIntercept(float slope, float voltage, float value);
//void calibrateRatio(float lowPower, float lowRatio, float highPower, float highRatio);

#endif /* _SWR_POWER_H_ */
