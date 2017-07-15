#include "swr_heartbeat.h"
#include "swr_constants.h"
#include <Arduino.h>

void heartbeatSetup() {
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
}

void heartbeatUpdate() {
  static boolean beatShort = true;
  static uint8_t beatCount = 0;
  static unsigned int  ledStatus = HIGH;  // Last set LED mode.
  static unsigned long ledBlinkTime = 0; // LED blink time.

  if ( (long)(millis() - ledBlinkTime) >= 0 )
  {
    if( beatShort ) {
      if( beatCount >= 2  ) {
        beatCount = 0;
        beatShort = !beatShort;
      }
      else
        beatCount++;
    }
    else if(!beatShort)
      beatShort = !beatShort;
      
    // Toggle LED.
    ledStatus = (ledStatus == HIGH ? LOW : HIGH);
    digitalWrite(HEARTBEAT_LED_PIN, ledStatus);

    // Reset "next time to toggle" time.
    ledBlinkTime = millis() + (beatShort ? HEARTBEAT_RATE_SHORT_MS : HEARTBEAT_RATE_LONG_MS);
  }
}
