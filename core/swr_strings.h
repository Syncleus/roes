#ifndef _SWR_STRINGS_H_
#define _SWR_STRINGS_H_

#include <avr/pgmspace.h>

#define MAX_STRING_LENGTH 32

#define CORRUPT_EEPROM 0
static const PROGMEM char CORRUPT_EEPROM_STRING[] = "EEPROM is corrupt";

#define CRC_CHECK_FAILED 1
static const PROGMEM char CRC_CHECK_FAILED_STRING[] = "CRC check failed";

static const PROGMEM char* const string_table[] PROGMEM = {
  CORRUPT_EEPROM_STRING, //0
  CRC_CHECK_FAILED_STRING //1
};

const char* strings(uint16_t id);
const char* strings(uint16_t id, const char *buffer);
void uint32toa(uint32_t value, char*buffer, uint8_t radix);

#endif /* _SWR_STRINGS_H_ */
