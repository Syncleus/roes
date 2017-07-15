#include "swr_heartbeat.h"
#include "swr_constants.h"
#include <Arduino.h>

void heartbeatSetup() {
  pinMode(HEARTBEAT_LED_PIN, OUTPUT);
}

void heartbeatUpdate() {
  static unsigned int  ledStatus = LOW;  // Last set LED mode.
  static unsigned long ledBlinkTime = 0; // LED blink time.

  if ( (long)(millis() - ledBlinkTime) >= 0 )
  {
    // Toggle LED.
    ledStatus = (ledStatus == HIGH ? LOW : HIGH);

    // Set LED pin status.
    digitalWrite(HEARTBEAT_LED_PIN, ledStatus);

    // Reset "next time to toggle" time.
    ledBlinkTime = millis() + HEARTBEAT_RATE_MS;
  }
}
