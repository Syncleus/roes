#include "swr_strings.h"
#include <Arduino.h>

const char* strings(uint16_t id) {
  static const char buffer[MAX_STRING_LENGTH];
  return strings(id, buffer);
}

const char* strings(uint16_t id, const char *buffer) {
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

