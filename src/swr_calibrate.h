#ifndef _SWR_CALIBRATE_H_
#define _SWR_CALIBRATE_H_

#include <Arduino.h>

struct CalibrationAverages {
  uint16_t adcFwd;
  uint16_t adcRvr;
  uint16_t adcVref;
  uint16_t adcMagnitude;
  uint16_t adcPhase;
  uint16_t adcPhaseShifted;
  uint16_t adcVrefShifted;
};

boolean runCalibration();
CalibrationAverages getCalibration();
boolean waitForStop();

#endif /* _SWR_CALIBRATE_H_ */
