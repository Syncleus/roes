#ifndef _SWR_DISPLAY_H_
#define _SWR_DISPLAY_H_

#include <Arduino.h>

void displaySetup();
float scaleToPercent(float value, float middle, float scale);
float scaleToPercent(float value, float value_min, float value_mid, float scale);
float logBased(float value, float base);
uint8_t percentBar(uint8_t y_offset, float percent);
String makeValueLabel(float value);
String makeValueLabel(float value, const char* units);
uint8_t renderCompleteBar(int8_t y_offset, const char* label, float value, const char* units, float value_min, float value_mid, float scale);
void renderPowerSwr(float power_fwd, float power_rvr);
void renderComplexSwr(float magnitude, float phase);
float swrFromPower(float power_fwd, float power_rvr);
void updatePowerDemo(float *power_fwd, float *power_rvr);
void updateComplexDemo(float *magnitudeDb, float *phase);
void renderError(String message1, String message2, String message3, String message4);
void renderCalibration(float power, boolean openLoad);
void renderStopTransmitting();
float dbToSwr(float magnitudeDb);
float polarToComplexA(float magnitudeDb, float phase);
float polarToComplexB(float magnitudeDb, float phase);

#endif /* _SWR_DISPLAY_H_ */
