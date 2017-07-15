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

etl::set<float, MAX_CALIBRATION_POWER_POINTS> calibrationPowerPoints();
void setCalibrationPowerPoints(etl::set<float, MAX_CALIBRATION_POWER_POINTS> newCalibrationPowerPoints);

int8_t powerPointToIndex(float powerPoint);
CalibrationData calibrationDataDummy(float powerPoint);
void setCalibrationDataDummy(float powerPoint, CalibrationData data);
CalibrationData calibrationDataOpen();
void setCalibrationDataOpen(CalibrationData data);

float lowestPowerPoint();
float highestPowerPoint();

#endif /* _SWR_EEPROM_H_ */
