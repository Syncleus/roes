#include "swr_eeprom.h"

#include <EEPROM.h>

#define EEPROM_CRC_ADDR 0
#define EEPROM_CRC_LENGTH 4
#define EEPROM_SIZE_ADDR (EEPROM_CRC_ADDR + EEPROM_CRC_LENGTH)
#define EEPROM_SIZE_LENGTH 4
#define EEPROM_DATA_ADDR (EEPROM_SIZE_ADDR + EEPROM_SIZE_LENGTH)

struct SwrPersistedData {
  boolean calibrateOnBoot;
  boolean demoMode;
  uint16_t calibrationLowFwd;
  uint16_t calibrationLowRvr;
  uint16_t calibrationLowVref;
  uint16_t calibrationLowMagnitude;
  uint16_t calibrationLowPhase;
  uint16_t calibrationHighFwd;
  uint16_t calibrationHighRvr;
  uint16_t calibrationHighVref;
  uint16_t calibrationHighMagnitude;
  uint16_t calibrationHighPhase;
  float calibrationLowRatio;
  float calibrationHighRatio;
};

SwrPersistedData persistedData;

uint32_t crc32(byte *data, int len) {
  const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  uint32_t crc = ~0L;

  //skip first 4 bytes which contain the CRC itself
  for (int index ; index < len  ; ++index) {
    crc = crc_table[(crc ^ data[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (data[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  
  return crc;
}

uint32_t eepromCrc32() {
  SwrPersistedData eepromData;
  EEPROM.get(EEPROM_DATA_ADDR, eepromData);
  return crc32((byte*) &eepromData, sizeof(eepromData));
}

uint32_t persistedDataCrc32() {
  return crc32((byte*) &persistedData, sizeof(persistedData));
}

//false
boolean checkEepromCrc() {
  uint32_t storedCrc;
  EEPROM.get(EEPROM_CRC_ADDR, storedCrc);
  uint32_t calculatedCrc = eepromCrc32();
  if( storedCrc == calculatedCrc )
    return true;
  else
    return false;
}

boolean storeData() {
  uint32_t crc = persistedDataCrc32();
  EEPROM.put(EEPROM_CRC_ADDR, crc);
  EEPROM.put(EEPROM_DATA_ADDR, persistedData);

  return crc == eepromCrc32();
}

boolean recallData() {
  EEPROM.get(EEPROM_DATA_ADDR, persistedData);
  return checkEepromCrc();
}

void eepromSetup() {
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

uint16_t calibrationLowFwd() {
  return persistedData.calibrationLowFwd;
}

void setCalibrationLowFwd(uint16_t adcValue) {
  if( persistedData.calibrationLowFwd == adcValue )
    return;

  persistedData.calibrationLowFwd = adcValue;
  storeData();
}

uint16_t calibrationLowRvr() {
  return persistedData.calibrationLowRvr;
}

void setCalibrationLowRvr(uint16_t adcValue) {
  if( persistedData.calibrationLowRvr == adcValue )
    return;

  persistedData.calibrationLowRvr = adcValue;
  storeData();
}

uint16_t calibrationHighFwd() {
  return persistedData.calibrationHighFwd;
}

void setCalibrationHighFwd(uint16_t adcValue) {
  if( persistedData.calibrationHighFwd == adcValue )
    return;

  persistedData.calibrationHighFwd = adcValue;
  storeData();
}

uint16_t calibrationHighRvr() {
  return persistedData.calibrationHighRvr;
}

void setCalibrationHighRvr(uint16_t adcValue) {
  if( persistedData.calibrationHighRvr == adcValue )
    return;

  persistedData.calibrationHighRvr = adcValue;
  storeData();
}

float calibrationHighRatio() {
  return persistedData.calibrationHighRatio;
}

void setCalibrationHighRatio(float ratio) {
  if( persistedData.calibrationHighRatio == ratio )
    return;

  persistedData.calibrationHighRatio = ratio;
  storeData();
}

float calibrationLowRatio() {
  return persistedData.calibrationLowRatio;
}

void setCalibrationLowRatio(float ratio) {
  if( persistedData.calibrationLowRatio == ratio )
    return;

  persistedData.calibrationLowRatio = ratio;
  storeData();
}

uint16_t calibrationLowVref() {
  return persistedData.calibrationLowVref;
}

void setCalibrationLowVref(uint16_t adcValue) {
  if( persistedData.calibrationLowVref == adcValue )
    return;

  persistedData.calibrationLowVref = adcValue;
  storeData();
}

uint16_t calibrationLowMagnitude() {
  return persistedData.calibrationLowMagnitude;
}

void setCalibrationLowMagnitude(uint16_t adcValue) {
  if( persistedData.calibrationLowMagnitude == adcValue )
    return;

  persistedData.calibrationLowMagnitude = adcValue;
  storeData();
}

uint16_t calibrationLowPhase() {
  return persistedData.calibrationLowPhase;
}

void setCalibrationLowPhase(uint16_t adcValue) {
  if( persistedData.calibrationLowPhase == adcValue )
    return;

  persistedData.calibrationLowPhase = adcValue;
  storeData();
}

uint16_t calibrationHighVref() {
  return persistedData.calibrationHighVref;
}

void setCalibrationHighVref(uint16_t adcValue) {
  if( persistedData.calibrationHighVref == adcValue )
    return;

  persistedData.calibrationHighVref = adcValue;
  storeData();
}

uint16_t calibrationHighMagnitude() {
  return persistedData.calibrationHighVref;
}

void setCalibrationHighMagnitude(uint16_t adcValue) {
  if( persistedData.calibrationHighMagnitude == adcValue )
    return;

  persistedData.calibrationHighMagnitude = adcValue;
  storeData();
}

uint16_t calibrationHighPhase() {
  return persistedData.calibrationHighPhase;
}

void setCalibrationHighPhase(uint16_t adcValue) {
  if( persistedData.calibrationHighPhase == adcValue )
    return;

  persistedData.calibrationHighPhase = adcValue;
  storeData();
}

