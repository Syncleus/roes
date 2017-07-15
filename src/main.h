#ifndef _MAIN_H_
#define _MAIN_H_

#include <Arduino.h>

void setup();
void loop();
void updateDownButton();
boolean bumpCalibratingPowerPoint();
void handleCalibrationData(char* tokens);
void handleReadInputs(char* tokens);
void handleCalibrateOnBoot(char* tokens);
void handlePing(char* tokens);
void handleDemo(char* tokens);
char* splitString(char* data, char separator);
void handleCalibrationPoints(char* tokens);
void handleHelp(char* tokens);
void handleClearEeprom(char* tokens);

#endif /* _MAIN_H_ */
