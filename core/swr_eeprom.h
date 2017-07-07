#ifndef _SWR_EEPROM_H_
#define _SWR_EEPROM_H_

#include <Arduino.h>

void eepromSetup();
boolean checkEepromCrc();
uint32_t eepromCrc32();
uint32_t persistedDataCrc32();
boolean calibrateOnBoot();
void activateCalibrateOnBoot();
void deactivateCalibrateOnBoot();
void activateDemoMode();
void deactivateDemoMode();
boolean demoMode();

uint16_t calibrationLowFwd();
void setCalibrationLowFwd(uint16_t adcValue);
uint16_t calibrationLowRvr();
void setCalibrationLowRvr(uint16_t adcValue);
uint16_t calibrationHighFwd();
void setCalibrationHighFwd(uint16_t adcValue);
uint16_t calibrationHighRvr();
void setCalibrationHighRvr(uint16_t adcValue);
float calibrationHighRatio();
void setCalibrationHighRatio(float ratio);
float calibrationLowRatio();
void setCalibrationLowRatio(float ratio);

#endif /* _SWR_EEPROM_H_ */
