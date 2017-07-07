#include "swr_calibrate.h"
#include "swr_constants.h"

Calibration calibration;

#define REQUIRED_SAMPLES 10000

#define THRESHOLD 50

uint32_t adcTotal = 0;
uint32_t samples = 0;

int32_t calibrate(uint8_t pin) {
  uint16_t adcValue = analogRead(pin);
  if( adcValue < THRESHOLD )
    return -1;

  adcTotal += adcValue;
  samples++;
  
  if(samples >= REQUIRED_SAMPLES) {
    int32_t calibrationResult = adcTotal / samples;
    adcTotal = 0;
    samples = 0;
    return calibrationResult;
  }
  else
    return -1;
}

boolean waitForStop(boolean forward) {
  uint16_t adcValue = analogRead(forward ? POWER_FWD_PIN : POWER_RVR_PIN);
  if( adcValue < THRESHOLD )
    adcTotal++;
  samples++;

  if(samples >= 100) {
    boolean returnVal = (adcTotal > 50);
    adcTotal = 0;
    samples = 0;
    return returnVal;
  }

  return false;
}

boolean calibrateLowFwd() {
  int32_t adcAverage = calibrate(POWER_FWD_PIN);
  if( adcAverage < 0 )
    return false;

  calibration.lowFwd = adcAverage;
  return true;
}

boolean calibrateLowRvr() {
  int32_t adcAverage = calibrate(POWER_RVR_PIN);
  if( adcAverage < 0 )
    return false;

  calibration.lowRvr = adcAverage;
  return true;
}

boolean calibrateHighFwd() {
  int32_t adcAverage = calibrate(POWER_FWD_PIN);
  if( adcAverage < 0 )
    return false;

  calibration.highFwd = adcAverage;
  return true;
}

boolean calibrateHighRvr() {
  int32_t adcAverage = calibrate(POWER_RVR_PIN);
  if( adcAverage < 0 )
    return false;

  calibration.highRvr = adcAverage;
  return true;
}

Calibration getCalibration() {
  return calibration;
}

