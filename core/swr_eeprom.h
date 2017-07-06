#ifndef _SWR_EEPROM_H_
#define _SWR_EEPROM_H_

#include <Arduino.h>

boolean calibrateOnBoot();
void activateCalibrateOnBoot();
void resetCalibrateOnBoot();
void eepromSetup();
boolean checkEepromCrc();
uint32_t eepromCrc32();
uint32_t persistedDataCrc32();

#endif /* _SWR_EEPROM_H_ */
