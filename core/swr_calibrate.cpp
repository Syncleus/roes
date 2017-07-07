#include "swr_calibrate.h"
#include "swr_constants.h"

#define REQUIRED_SAMPLES 10000

#define THRESHOLD 10

Calibration calibration;
uint32_t adcFwdTotal = 0;
uint32_t adcRvrTotal = 0;
uint32_t adcVrefTotal = 0;
uint32_t adcMagnitudeTotal = 0;
uint32_t adcPhaseTotal = 0;
uint32_t samples = 0;

struct Averages {
  int32_t adcFwd;
  int32_t adcRvr;
  int32_t adcVref;
  int32_t adcMagnitude;
  int32_t adcPhase;
};

Averages calibrate() {
  Averages avg = { -1, -1, -1, -1, -1};
  uint16_t adcFwdValue = analogRead(POWER_FWD_PIN);
  uint16_t adcRvrValue = analogRead(POWER_RVR_PIN);
  uint16_t adcVrefValue = analogRead(COMPLEX_VREF_PIN);
  uint16_t adcMagnitudeValue = analogRead(COMPLEX_MAGNITUDE_PIN);
  uint16_t adcPhaseValue = analogRead(COMPLEX_PHASE_PIN);
  if( (adcFwdValue < THRESHOLD) && ( adcRvrValue < THRESHOLD ) )
    return avg;

  adcFwdTotal += adcFwdValue;
  adcRvrTotal += adcRvrValue;
  adcVrefTotal += adcVrefValue;
  adcPhaseTotal += adcPhaseValue;
  adcMagnitudeTotal += adcMagnitudeValue;
  samples++;
  
  if(samples >= REQUIRED_SAMPLES) {
    avg.adcFwd = adcFwdTotal / samples;
    avg.adcRvr = adcRvrTotal / samples;
    avg.adcVref = adcVrefTotal / samples;
    avg.adcMagnitude = adcMagnitudeTotal / samples;
    avg.adcPhase = adcPhaseTotal / samples;
    adcFwdTotal = 0;
    adcRvrTotal = 0;
    uint32_t adcVrefTotal = 0;
    uint32_t adcMagnitudeTotal = 0;
    uint32_t adcPhaseTotal = 0;
    samples = 0;
  }
  
  return avg;
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
    return returnVal;
  }

  return false;
}

boolean calibrateLowFwd() {
  Averages avg = calibrate();
  if( avg.adcFwd < 0 )
    return false;

  calibration.lowFwd = avg.adcFwd;
  calibration.lowVref = avg.adcVref;
  calibration.lowMagnitude = avg.adcMagnitude;
  calibration.lowPhase = avg.adcPhase;
  calibration.lowRatio = ((float)avg.adcFwd) / ((float)avg.adcRvr);
  return true;
}

boolean calibrateLowRvr() {
  Averages avg = calibrate();
  if( avg.adcRvr < 0 )
    return false;

  calibration.lowRvr = avg.adcRvr;
  return true;
}

boolean calibrateHighFwd() {
  Averages avg = calibrate();
  if( avg.adcFwd < 0 )
    return false;

  calibration.highFwd = avg.adcFwd;
  calibration.highVref = avg.adcVref;
  calibration.highMagnitude = avg.adcMagnitude;
  calibration.highPhase = avg.adcPhase;
  calibration.highRatio = ((float)avg.adcFwd) / ((float)avg.adcRvr);
  return true;
}

boolean calibrateHighRvr() {
  Averages avg = calibrate();
  if( avg.adcRvr < 0 )
    return false;

  calibration.highRvr = avg.adcRvr;
  return true;
}

Calibration getCalibration() {
  return calibration;
}

