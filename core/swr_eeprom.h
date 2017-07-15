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
uint32_t eepromCrc32();
uint32_t persistedDataCrc32();
boolean calibrateOnBoot();
void activateCalibrateOnBoot();
void deactivateCalibrateOnBoot();
void activateDemoMode();
void deactivateDemoMode();
boolean demoMode();

etl::set<float, MAX_CALIBRATION_POWER_POINTS> calibrationPowerPoints();
void setCalibrationPowerPoints(etl::set<float, MAX_CALIBRATION_POWER_POINTS> newCalibrationPowerPoints);

uint8_t bandToIndex(char* band);
uint8_t powerPointToIndex(float powerPoint);
CalibrationData calibrationDataDummy(float powerPoint);
void setCalibrationDataDummy(float powerPoint, CalibrationData data);
uint16_t calibrationDataOpen();
void setCalibrationDataOpen(uint16_t data);

#endif /* _SWR_EEPROM_H_ */
