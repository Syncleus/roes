#include "swr_strings.h"
#include <Arduino.h>

static char sharedBuffer[BUFFER_COUNT][MAX_STRING_LENGTH];
static uint8_t currentBuffer = 0;

const char* strings(uint16_t id) {
  currentBuffer++;
  if( currentBuffer >= BUFFER_COUNT )
    currentBuffer = 0;
  return strings(id, sharedBuffer[currentBuffer]);
}

char* strings(uint16_t id, char *buffer) {
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[id])));
  return buffer;
}

void uint32toa(uint32_t value, char* buffer, uint8_t radix) {
  uint16_t valueUpper = value>>16;
  uint16_t valueLower = value;
  itoa(valueUpper, buffer, radix);
  itoa(valueLower, buffer + strlen(buffer), radix);

  while (*buffer) {
    *buffer = toupper((unsigned char) *buffer);
    buffer++;
  }
}
