#ifndef _SWR_COMMANDLINE_H_
#define _SWR_COMMANDLINE_H_

void commandlineSetup();
void commandlineUpdate();
void handleCalibrationData(char* tokens);
void handleReadInputs(char* tokens);
void handleCalibrateOnBoot(char* tokens);
void handlePing(char* tokens);
void handleDemo(char* tokens);
void handleCalibrationPoints(char* tokens);
void handleHelp(char* tokens);
void handleClearEeprom(char* tokens);
void handleSwrSource(char* tokens);

#endif /* _SWR_COMMANDLINE_H_ */
