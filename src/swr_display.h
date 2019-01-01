#ifndef _SWR_DISPLAY_H_
#define _SWR_DISPLAY_H_

#include <Arduino.h>

void displaySetup();
float scaleToPercent(float value, float middle, float scale);
float scaleToPercent(float value, float value_min, float value_mid, float scale);
uint8_t percentBar(uint8_t y_offset, float percent);
String makeValueLabel(float value);
String makeValueLabel(float value, const char* units);
uint8_t renderCompleteBar(int8_t y_offset, const char* label, float value, const char* units, float value_min, float value_mid, float scale);
uint8_t renderCompleteBar(int8_t y_offset, const char* label, float value, const char* units, float value_min, float value_mid, float scale, boolean inverse);
void updatePowerDemo(float *power_fwd, float *power_rvr);
void updateComplexDemo(float *magnitudeDb, float *phase);
void renderError(String message1, String message2, String message3, String message4);

void prepareRender();
void finishRender();
void renderSwr(float swr);
void renderPowerBars(float power_fwd, float power_rvr);
void renderReflectionBars(float magnitude, float phase);
void renderStopTransmitting();
void renderCalibration(float power, boolean dummyLoad);
void renderPowerText(float power_fwd, float power_rvr);
void renderLoadText(float magnitudeDb, float phase);
void renderSmithChart(float reflMagDb, float reflPhase, float loatMagDb, float loadPhase);

#endif /* _SWR_DISPLAY_H_ */
