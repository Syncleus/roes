#ifndef _SWR_STRINGS_H_
#define _SWR_STRINGS_H_

#include <avr/pgmspace.h>

#define MAX_STRING_LENGTH 32
#define BUFFER_COUNT 4

#define CORRUPT_EEPROM 0
static const PROGMEM char CORRUPT_EEPROM_STRING[] = "EEPROM is corrupt";

#define CRC_CHECK_FAILED 1
static const PROGMEM char CRC_CHECK_FAILED_STRING[] = "CRC check failed";

#define SWR_LABEL 2
static const PROGMEM char SWR_LABEL_STRING[] = "SWR";

#define FWD_LABEL 3
static const PROGMEM char FWD_LABEL_STRING[] = "Fwd";

#define RVR_LABEL 4
static const PROGMEM char RVR_LABEL_STRING[] = "Rvr";

#define MAG_LABEL 5
static const PROGMEM char MAG_LABEL_STRING[] = "Mag";

#define PHS_LABEL 6
static const PROGMEM char PHS_LABEL_STRING[] = "Phs";

#define WATTS_UNIT_LABEL 7
static const PROGMEM char WATTS_UNIT_LABEL_STRING[] = "w";

#define DECIBEL_UNIT_LABEL 8
static const PROGMEM char DECIBEL_UNIT_LABEL_STRING[] = "dB";

#define STOP_WARNING_LABEL 9
static const PROGMEM char STOP_WARNING_LABEL_STRING[] = "STOP";

#define TRANSMITTING_LABEL 10
static const PROGMEM char TRANSMITTING_LABEL_STRING[] = "transmitting";

#define CALIBRATE_LABEL 11
static const PROGMEM char CALIBRATE_LABEL_STRING[] = "Calibrate";

#define CALIBRATE_LINE_1A 12
static const PROGMEM char CALIBRATE_LINE_1A_STRING[] = "Apply ";

#define CALIBRATE_LINE_1B 13
static const PROGMEM char CALIBRATE_LINE_1B_STRING[] = " into";

#define CALIBRATE_LINE_2_OPEN 14
static const PROGMEM char CALIBRATE_LINE_2_OPEN_STRING[] = "an open load";

#define CALIBRATE_LINE_2_DUMMY 15
static const PROGMEM char CALIBRATE_LINE_2_DUMMY_STRING[] = "a dummy load";

#define ERROR_WARNING_LABEL 16
static const PROGMEM char ERROR_WARNING_LABEL_STRING[] = "ERROR!";

static const PROGMEM char* const string_table[] PROGMEM = {
  CORRUPT_EEPROM_STRING, //0
  CRC_CHECK_FAILED_STRING, //1
  SWR_LABEL_STRING, //2
  FWD_LABEL_STRING, //3
  RVR_LABEL_STRING, //4
  MAG_LABEL_STRING, //5
  PHS_LABEL_STRING,  //6
  WATTS_UNIT_LABEL_STRING, //7
  DECIBEL_UNIT_LABEL_STRING, //8
  STOP_WARNING_LABEL_STRING, //9
  TRANSMITTING_LABEL_STRING, //10
  CALIBRATE_LABEL_STRING, //11
  CALIBRATE_LINE_1A_STRING, //12
  CALIBRATE_LINE_1B_STRING, //13
  CALIBRATE_LINE_2_OPEN_STRING, //14
  CALIBRATE_LINE_2_DUMMY_STRING, //15
  ERROR_WARNING_LABEL_STRING //16
};

const char* strings(uint16_t id);
char* strings(uint16_t id, char *buffer);
void uint32toa(uint32_t value, char *buffer, uint8_t radix);

#endif /* _SWR_STRINGS_H_ */
