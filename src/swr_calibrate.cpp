#include "swr_calibrate.h"
#include "swr_constants.h"

struct CalibrationPoint {
  String band;
  float power;
};

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
  uint16_t adcRvrValue = analogRead(POWER_REFL_PIN);
  uint16_t adcVrefValue = analogRead(DIFFERENTIAL_VREF_PIN);
  uint16_t adcMagnitudeValue = analogRead(DIFFERENTIAL_MAGNITUDE_PIN);
  uint16_t adcPhaseValue = analogRead(DIFFERENTIAL_PHASE_PIN);
  if( (adcFwdValue < CALIBRATION_THRESHOLD) && ( adcRvrValue < CALIBRATION_THRESHOLD ) )
    return;

  adcFwdTotal += adcFwdValue;
  adcRvrTotal += adcRvrValue;
  adcVrefTotal += adcVrefValue;
  adcPhaseTotal += adcPhaseValue;
  adcMagnitudeTotal += adcMagnitudeValue;
  samples++;

  if(samples >= CALIBRATION_SAMPLES) {
    averages.adcFwd = adcFwdTotal / samples;
    averages.adcRvr = adcRvrTotal / samples;
    averages.adcVref = adcVrefTotal / samples;
    averages.adcMagnitude = adcMagnitudeTotal / samples;
    averages.adcPhase = adcPhaseTotal / samples;
    adcFwdTotal = 0;
    adcRvrTotal = 0;
    adcVrefTotal = 0;
    adcMagnitudeTotal = 0;
    adcPhaseTotal = 0;
    samples = 0;
    calibrationComplete = true;
  }
}

boolean waitForStop() {
  uint16_t adcValue = analogRead(POWER_FWD_PIN);
  if( adcValue < 2 )
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
