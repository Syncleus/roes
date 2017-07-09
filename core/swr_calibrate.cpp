#include "swr_calibrate.h"
#include "swr_constants.h"

struct CalibrationPoint {
  String band;
  float power;
};

#define REQUIRED_SAMPLES 10000
#define THRESHOLD 10

CalibrationAverages averages;
boolean calibrationComplete = false;
uint32_t adcFwdTotal = 0;
uint32_t adcRvrTotal = 0;
uint32_t adcVrefTotal = 0;
uint32_t adcMagnitudeTotal = 0;
uint32_t adcPhaseTotal = 0;
uint32_t samples = 0;

void calibrate() {
  calibrationComplete = false;
  uint16_t adcFwdValue = analogRead(POWER_FWD_PIN);
  uint16_t adcRvrValue = analogRead(POWER_RVR_PIN);
  uint16_t adcVrefValue = analogRead(COMPLEX_VREF_PIN);
  uint16_t adcMagnitudeValue = analogRead(COMPLEX_MAGNITUDE_PIN);
  uint16_t adcPhaseValue = analogRead(COMPLEX_PHASE_PIN);
  if( (adcFwdValue < THRESHOLD) && ( adcRvrValue < THRESHOLD ) )
    return;

  adcFwdTotal += adcFwdValue;
  adcRvrTotal += adcRvrValue;
  adcVrefTotal += adcVrefValue;
  adcPhaseTotal += adcPhaseValue;
  adcMagnitudeTotal += adcMagnitudeValue;
  samples++;
  
  if(samples >= REQUIRED_SAMPLES) {
    averages.adcFwd = adcFwdTotal / samples;
    averages.adcRvr = adcRvrTotal / samples;
    averages.adcVref = adcVrefTotal / samples;
    averages.adcMagnitude = adcMagnitudeTotal / samples;
    averages.adcPhase = adcPhaseTotal / samples;
    adcFwdTotal = 0;
    adcRvrTotal = 0;
    uint32_t adcVrefTotal = 0;
    uint32_t adcMagnitudeTotal = 0;
    uint32_t adcPhaseTotal = 0;
    samples = 0;
    calibrationComplete = true;
  }
}

boolean waitForStop(boolean forward) {
  uint16_t adcValue = analogRead(forward ? POWER_FWD_PIN : POWER_RVR_PIN);
  if( adcValue < THRESHOLD )
    adcFwdTotal++;
  samples++;

  if(samples >= 100) {
    boolean returnVal = (adcFwdTotal > 50);
    adcFwdTotal = 0;
    samples = 0;
    calibrationComplete = false;
    return returnVal;
  }

  return false;
}

boolean runCalibration() {
  calibrate();
  if( !calibrationComplete )
    return false;
  else
    return true;
}

CalibrationAverages getCalibration() {
  return averages;
}

