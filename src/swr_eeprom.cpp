#include "swr_eeprom.h"
#include <cstring.h>
#include <map.h>
#include <container.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

#define EEPROM_CRC_ADDR 0
#define EEPROM_CRC_LENGTH 4
#define EEPROM_DATA_ADDR (EEPROM_CRC_ADDR + EEPROM_CRC_LENGTH)

struct SwrPersistedData {
  boolean calibrateOnBoot;
  boolean demoMode;
  float calibrationPowerPointsDummy[MAX_CALIBRATION_POWER_POINTS_DUMMY];
  float calibrationPowerPointsOpen[MAX_CALIBRATION_POWER_POINTS_OPEN];
  CalibrationData calibrationDataDummy[MAX_CALIBRATION_POWER_POINTS_DUMMY];
  CalibrationData calibrationDataOpen;
};

SwrPersistedData persistedData;

void eepromClear() {
  for (uint16_t i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.update(i, 0);
  }
}

boolean isEepromBlank() {
  uint32_t storedCrc;
  EEPROM.get(EEPROM_CRC_ADDR, storedCrc);
  return storedCrc == 0;
}

static PROGMEM uint32_t crc32(byte *data, int len) {
  const static PROGMEM uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  uint32_t crc = ~0L;

  //skip first 4 bytes which contain the CRC itself
  for (int index ; index < len  ; ++index) {
    uint32_t data_read1 = pgm_read_dword_near(crc_table + ((crc ^ data[index]) & 0x0f));
    uint32_t data_read2 = pgm_read_dword_near(crc_table + (crc ^ (data[index] >> 4)));
    crc = data_read1 ^ (crc >> 4);
    crc = data_read2 ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}

uint32_t eepromCrc32Actual() {
  SwrPersistedData eepromData;
  EEPROM.get(EEPROM_DATA_ADDR, eepromData);
  return crc32((byte*) &eepromData, sizeof(eepromData));
}

uint32_t eepromCrc32Stored() {
  uint32_t storedCrc;
  EEPROM.get(EEPROM_CRC_ADDR, storedCrc);
  return storedCrc;
}

uint32_t persistedDataCrc32() {
  return crc32((byte*) &persistedData, sizeof(persistedData));
}

boolean checkEepromCrc() {
  uint32_t storedCrc = eepromCrc32Stored();
  uint32_t calculatedCrc = eepromCrc32Actual();
  return (storedCrc == calculatedCrc);
}

boolean storeData() {
  uint32_t crc = persistedDataCrc32();
  EEPROM.put(EEPROM_CRC_ADDR, crc);
  if( sizeof(persistedData) + EEPROM_CRC_LENGTH < EEPROM.length() )
    EEPROM.put(EEPROM_DATA_ADDR, persistedData);

  return crc == eepromCrc32Actual();
}

boolean recallData() {
  EEPROM.get(EEPROM_DATA_ADDR, persistedData);
  return checkEepromCrc();
}

void eepromSetup() {
  if( isEepromBlank() )
  {
    for(int index; index < MAX_CALIBRATION_POWER_POINTS_DUMMY; index++)
      persistedData.calibrationPowerPointsDummy[index] = -1.0;
    for(int indexPoint; indexPoint < MAX_CALIBRATION_POWER_POINTS_DUMMY; indexPoint++)
      persistedData.calibrationDataDummy[indexPoint] = {0, 0, 0};
    storeData();


  }
  else
    recallData();
}

boolean calibrateOnBoot() {
  return persistedData.calibrateOnBoot;
}

void activateCalibrateOnBoot() {
  if( persistedData.calibrateOnBoot == true )
    return;

  persistedData.calibrateOnBoot = true;
  storeData();
}

void deactivateCalibrateOnBoot() {
  if( persistedData.calibrateOnBoot == false )
    return;

  persistedData.calibrateOnBoot = false;
  storeData();
}

boolean demoMode() {
  return persistedData.demoMode;
}

void activateDemoMode() {
  if( persistedData.demoMode == true )
    return;

  persistedData.demoMode = true;
  storeData();
}

void deactivateDemoMode() {
  if( persistedData.demoMode == false )
    return;

  persistedData.demoMode = false;
  storeData();
}

etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPowerPointsDummy() {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPowerPointsDummySet;
  for(uint16_t index = 0; index < MAX_CALIBRATION_POWER_POINTS_DUMMY; index++) {
    float calibrationPowerPoint = persistedData.calibrationPowerPointsDummy[index];
    if( calibrationPowerPoint > 0.0 )
      calibrationPowerPointsDummySet.insert(calibrationPowerPoint);
  }
  return calibrationPowerPointsDummySet;
}

void setcalibrationPowerPointsDummy(etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> newcalibrationPowerPointsDummy) {
  etl::iset<float, std::less<float>>::const_iterator itr;

  //clear current array
  for(int index = 0; index < MAX_CALIBRATION_POWER_POINTS_DUMMY; index++)
    persistedData.calibrationPowerPointsDummy[index] = -1.0;

  itr = newcalibrationPowerPointsDummy.begin();
  int index = 0;
  while (itr != newcalibrationPowerPointsDummy.end())
  {
    float currentCalibrationPowerPoint = *itr++;
    persistedData.calibrationPowerPointsDummy[index] = currentCalibrationPowerPoint;
    index++;
  }
  storeData();
}

etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPowerPointsOpen() {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPowerPointsOpenSet;
  for(uint16_t index = 0; index < MAX_CALIBRATION_POWER_POINTS_OPEN; index++) {
    float calibrationPowerPoint = persistedData.calibrationPowerPointsOpen[index];
    if( calibrationPowerPoint > 0.0 )
      calibrationPowerPointsOpenSet.insert(calibrationPowerPoint);
  }
  return calibrationPowerPointsOpenSet;
}

void setcalibrationPowerPointsOpen(etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> newcalibrationPowerPointsOpen) {
  etl::iset<float, std::less<float>>::const_iterator itr;

  //clear current array
  for(int index = 0; index < MAX_CALIBRATION_POWER_POINTS_OPEN; index++)
    persistedData.calibrationPowerPointsOpen[index] = -1.0;

  itr = newcalibrationPowerPointsOpen.begin();
  int index = 0;
  while (itr != newcalibrationPowerPointsOpen.end())
  {
    float currentCalibrationPowerPoint = *itr++;
    persistedData.calibrationPowerPointsOpen[index] = currentCalibrationPowerPoint;
    index++;
  }
  storeData();
}

int8_t powerPointToIndex(float powerPoint) {
  for(int index = 0; index < MAX_CALIBRATION_POWER_POINTS_DUMMY; index++) {
    if( persistedData.calibrationPowerPointsDummy[index] == powerPoint )
      return index;
  }
  return -1;
}

CalibrationData calibrationDataDummy(float powerPoint) {
  uint8_t powerPointIndex = powerPointToIndex(powerPoint);
  return persistedData.calibrationDataDummy[powerPointIndex];
}

void setCalibrationDataDummy(float powerPoint, CalibrationData data) {
  uint8_t powerPointIndex = powerPointToIndex(powerPoint);
  persistedData.calibrationDataDummy[powerPointIndex] = data;

  storeData();
}

CalibrationData calibrationDataOpen() {
  return persistedData.calibrationDataOpen;
}

void setCalibrationDataOpen(CalibrationData data) {
  persistedData.calibrationDataOpen = data;

  storeData();
}

float lowestPowerPoint() {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPoints = calibrationPowerPointsDummy();
  etl::iset<float, std::less<float>>::const_iterator itr = powerPoints.begin();
  float power = -1.0;
  while (itr != powerPoints.end())
  {
    float powerPoint = *itr++;
    if(powerPoint < power || power == -1.0)
      power = powerPoint;
  }
  return power;
}

float highestPowerPoint() {
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPoints = calibrationPowerPointsDummy();
  etl::iset<float, std::less<float>>::const_iterator itr = powerPoints.begin();
  float power = -1.0;
  while (itr != powerPoints.end())
  {
    float powerPoint = *itr++;
    if(powerPoint > power)
      power = powerPoint;
  }
  return power;
}
