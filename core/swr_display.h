#ifndef _SWR_DISPLAY_H_
#define _SWR_DISPLAY_H_

#include <Arduino.h>

void displaySetup();
float scaleToPercent(float value, float middle, float scale);
float scaleToPercent(float value, float value_min, float value_mid, float scale);
float logBased(float value, float base);
uint8_t percentBar(uint8_t y_offset, float percent);
String makeValueLabel(float value, String units);
void renderCompleteBar(int8_t y_offset, String label, float value, String units, float value_min, float value_mid, float scale);
void render(float power_fwd, float power_rvr);
float swrFromPower(float power_fwd, float power_rvr);
void updatePowerDemo(float &power_fwd, float &power_rvr);

#endif /* _SWR_DISPLAY_H_ */
