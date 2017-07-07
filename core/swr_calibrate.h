#ifndef _SWR_CALIBRATE_H_
#define _SWR_CALIBRATE_H_

#include <Arduino.h>

struct Calibration {
  uint16_t lowFwd;
  uint16_t lowRvr;
  float lowRatio;
  uint16_t highFwd;
  uint16_t highRvr;
  float highRatio;
};

boolean calibrateLowFwd();
boolean calibrateLowRvr();
boolean calibrateHighFwd();
boolean calibrateHighRvr();
Calibration getCalibration();
boolean waitForStop(boolean forward);

#endif /* _SWR_CALIBRATE_H_ */
