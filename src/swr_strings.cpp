#include "swr_strings.h"
#include <Arduino.h>

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

//splits a string in place, no dynamic allocation
char* splitString(char* data, char separator) {
  int dataIndex = -1;
  char currentChar = '\0';
  do {
    dataIndex++;
    currentChar = data[dataIndex];
  } while(currentChar != '\0' && currentChar != separator);

  if( currentChar == '\0' )
    return NULL;

  data[dataIndex] = '\0';
  return data + dataIndex + 1;
}
