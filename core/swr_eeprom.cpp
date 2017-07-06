#include "swr_eeprom.h"

#include <EEPROM.h>

#define EEPROM_CRC_ADDR 0
#define EEPROM_CRC_LENGTH 4

struct SwrPersistedData {
  boolean calibrateOnBoot;
};

SwrPersistedData persistedData = {
  false
};

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
  byte *eepromData = new byte[EEPROM.length() - EEPROM_CRC_LENGTH];
  EEPROM.get(EEPROM_CRC_ADDR + EEPROM_CRC_LENGTH, eepromData);
  return crc32(eepromData, EEPROM.length() - EEPROM_CRC_LENGTH);
}

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
  byte *dataAsArray = (byte*) &persistedData;
  int dataSize = sizeof(persistedData);
  uint32_t crc = crc32(dataAsArray, dataSize);
  EEPROM.put(EEPROM_CRC_ADDR, crc);
  EEPROM.put(EEPROM_CRC_ADDR + EEPROM_CRC_LENGTH, persistedData);

  return checkEepromCrc();
}

boolean recallData() {
  EEPROM.get(EEPROM_CRC_ADDR + EEPROM_CRC_LENGTH, persistedData);

  return checkEepromCrc();
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

void resetCalibrateOnBoot() {
  if( persistedData.calibrateOnBoot == false )
    return;

  persistedData.calibrateOnBoot = false;
  storeData();
}

void eepromSetup() {
  recallData();
}

