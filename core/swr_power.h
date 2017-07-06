#ifndef _SWR_POWER_H_
#define _SWR_POWER_H_

#include <Arduino.h>

void powerSetup();
float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc);
float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc);
void calibrateFwd(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc);
void calibrateRvr(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc);
float calculateFwdPower(uint16_t adcValue);
float calculateRvrPower(uint16_t adcValue);
float voltageToPower(float voltage);
float powerToVoltage(float power);
float readPowerFwd();
float readPowerRvr();
void updatePower(float &power_fwd, float &power_rvr);

#endif /* _SWR_POWER_H_ */
