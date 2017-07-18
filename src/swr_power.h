#ifndef _SWR_POWER_H_
#define _SWR_POWER_H_

#include <Arduino.h>
#include <complex.h>

struct PowerPointBounds {
  float lowVoltagePoint;
  float highVoltagePoint;
  uint16_t lowAdcValue;
  uint16_t highAdcValue;
  boolean outOfBounds;
};

PowerPointBounds determineBounds(uint16_t adcValue, boolean dummy);
float adcToVoltage(uint16_t adcValue, boolean forward);
uint16_t voltageToAdc(float voltage, boolean forward);
float voltageToPower(float voltage);
float powerToVoltage(float power);
void updatePower(float *power_fwd, float *power_rvr);
void updateReflection(float *magnitudeDb, float *phase);
float lowestPowerPoint(boolean dummy);
float highestPowerPoint(boolean dummy);
float logBased(float value, float base);
float powerToSwr(float power_fwd, float power_rvr);
float dbToSwr(float magnitudeDb);
Complex polarToComplex(float magnitude, float phase);
float powerToDbm(float power);
Complex complexLoadFromReflection(float magnitude, float phase);

#endif /* _SWR_POWER_H_ */
