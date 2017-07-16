#ifndef _SWR_EEPROM_H_
#define _SWR_EEPROM_H_

#include <Arduino.h>
#include <ArduinoSTL.h>
#include <set.h>
#include "swr_constants.h"

struct CalibrationData {
  uint16_t fwd;
  uint16_t rvr;
  uint16_t vref;
  uint16_t magnitude;
  uint16_t phase;
};

void eepromSetup();
void eepromClear();
boolean isEepromBlank();
boolean checkEepromCrc();
uint32_t persistedDataCrc32();
uint32_t eepromCrc32Actual();
uint32_t eepromCrc32Stored();
boolean calibrateOnBoot();
void activateCalibrateOnBoot();
void deactivateCalibrateOnBoot();
void activateDemoMode();
void deactivateDemoMode();
boolean demoMode();

etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPowerPointsDummy();
void setCalibrationPowerPointsDummy(etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> newcalibrationPowerPointsDummy);
etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPowerPointsOpen();
void setCalibrationPowerPointsOpen(etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> newcalibrationPowerPointsOpen);


int8_t powerPointToIndex(float powerPoint, boolean dummy);
CalibrationData calibrationData(float powerPoint, boolean dummy);
void setCalibrationData(float powerPoint, boolean dummy, CalibrationData data);

#endif /* _SWR_EEPROM_H_ */
