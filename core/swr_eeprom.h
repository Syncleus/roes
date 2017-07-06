#ifndef _SWR_EEPROM_H_
#define _SWR_EEPROM_H_

#include <Arduino.h>

boolean calibrateOnBoot();
void activateCalibrateOnBoot();
void resetCalibrateOnBoot();
void eepromSetup();
boolean checkEepromCrc();

#endif /* _SWR_EEPROM_H_ */
