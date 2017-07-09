#ifndef _SWR_EEPROM_H_
#define _SWR_EEPROM_H_

#include <Arduino.h>
#include <ArduinoSTL.h>
#include <set.h>
#include "swr_constants.h"

struct CalibrationData {
  uint16_t fwd;
  uint16_t rvr;
  uint16_t fwdRefl;
};

void eepromSetup();
void eepromClear();
boolean isEepromBlank();
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
uint16_t calibrationLowVref();
void setCalibrationLowVref(uint16_t adcValue);
uint16_t calibrationLowMagnitude();
void setCalibrationLowMagnitude(uint16_t adcValue);
uint16_t calibrationLowPhase();
void setCalibrationLowPhase(uint16_t adcValue);
float calibrationLowRatio();
void setCalibrationLowRatio(float ratio);

uint16_t calibrationHighFwd();
void setCalibrationHighFwd(uint16_t adcValue);
uint16_t calibrationHighRvr();
void setCalibrationHighRvr(uint16_t adcValue);
uint16_t calibrationHighVref();
void setCalibrationHighVref(uint16_t adcValue);
uint16_t calibrationHighMagnitude();
void setCalibrationHighMagnitude(uint16_t adcValue);
uint16_t calibrationHighPhase();
void setCalibrationHighPhase(uint16_t adcValue);
float calibrationHighRatio();
void setCalibrationHighRatio(float ratio);

etl::set<String, MAX_BANDS_COUNT> bands();
void setBands(etl::set<String, MAX_BANDS_COUNT> newBands);
etl::set<float, MAX_CALIBRATION_POWER_POINTS> calibrationPowerPoints();
void setCalibrationPowerPoints(etl::set<float, MAX_CALIBRATION_POWER_POINTS> newCalibrationPowerPoints);

uint8_t bandToIndex(char* band);
uint8_t powerPointToIndex(float powerPoint);
CalibrationData calibrationData(char* band, float powerPoint);
void setCalibrationData(char* band, float powerPoint, CalibrationData data);

#endif /* _SWR_EEPROM_H_ */
