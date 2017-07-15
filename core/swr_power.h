#ifndef _SWR_POWER_H_
#define _SWR_POWER_H_

#include <Arduino.h>

float adcToFwdVoltage(uint16_t adcValue);
uint16_t fwdVoltageToAdc(float voltage);
float adcToRvrVoltage(uint16_t adcValue);
uint16_t RvrVoltageToAdc(float voltage);
float fwdVoltageToRatio(float voltage);
float ratioToFwdVoltage(float ratio);
float voltageToPower(float voltage);
float powerToVoltage(float power);
void updatePower(float *power_fwd, float *power_rvr);
void updateComplex(float *magnitude, float *phase);

#endif /* _SWR_POWER_H_ */
