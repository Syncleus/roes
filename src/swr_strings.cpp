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
