#include "swr_status_led.h"
#include "swr_constants.h"
#include <Arduino.h>

LedStatus ledStatus = OFF;
unsigned long ledBlinkTime = 0; // LED blink time.

void statusLedSetup() {
  pinMode(STATUS_LED_PIN, OUTPUT);
}

void statusLedUpdate() {
  static unsigned int  ledState = HIGH;  // Last set LED mode.

  if ( (long)(millis() - ledBlinkTime) >= 0 )
  {
      
    // Toggle LED.
    ledState = (ledState == HIGH ? LOW : HIGH);

    uint16_t beatTime = LED_STATUS_RATE_SLOW_MS;
    switch(ledStatus) {
    case OFF:
      ledState = LOW;
      break;
    case ON:
      ledState = HIGH;
      break;
    case SLOW:
      break;
    case FAST:
      beatTime = LED_STATUS_RATE_FAST_MS;
      break;
    case VERY_FAST:
      beatTime = LED_STATUS_RATE_VERY_FAST_MS;
      break;
    }
    
    ledBlinkTime = millis() + beatTime;
    digitalWrite(STATUS_LED_PIN, ledState);
  }
}

void setLedStatus(LedStatus newStatus) {
  if( ledStatus != newStatus ) {
    ledStatus = newStatus;
    ledBlinkTime = 0;
  }
}

