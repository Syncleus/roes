#include "swr_commandline.h"
#include "swr_eeprom.h"
#include "swr_strings.h"

#include <Arduino.h>
#include <CommandLine.h>
#include <ArduinoSTL.h>

CommandLine commandLine(Serial, "> ");

void commandlineSetup() {
  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
  commandLine.add("calibrationpoints", handleCalibrationPoints);
  commandLine.add("cleareeprom", handleClearEeprom);
  commandLine.add("calibrateonboot", handleCalibrateOnBoot);
  commandLine.add("calibrationdata", handleCalibrationData);
  commandLine.add("readinputs", handleReadInputs);
}

void commandlineUpdate() {
  commandLine.update();
}

void handleCalibrationPoints(char* tokens) {
  char* argument = strtok(NULL, "\0");
  if( argument == NULL ) {
    etl::iset<float, std::less<float>>::const_iterator itr;

    etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPointsDataDummy = calibrationPowerPointsDummy();
    // Iterate through the list.
    itr = calibrationPointsDataDummy.begin();
    Serial.print("calibration points [dummy]: ");
    while (itr != calibrationPointsDataDummy.end())
    {
      Serial.print(String(*itr++));
      Serial.print(" ");
    }
    Serial.println();

    etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsDataOpen = calibrationPowerPointsOpen();
    // Iterate through the list.
    itr = calibrationPointsDataOpen.begin();
    Serial.print("calibration points [open]: ");
    while (itr != calibrationPointsDataOpen.end())
    {
      Serial.print(String(*itr++));
      Serial.print(" ");
    }
    Serial.println();
  }
  else {
    char* parsedArgument = argument;
    argument = splitString(parsedArgument, ' ');
    boolean isDummyPoints = true;
    if( String(parsedArgument).equals("open") )
      isDummyPoints = false;

    if( isDummyPoints ) {
      etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPointsData;
      do {
        parsedArgument = argument;
        argument = splitString(parsedArgument, ' ');
        calibrationPointsData.insert(String(parsedArgument).toFloat());
      } while(argument != NULL);

      setCalibrationPowerPointsDummy(calibrationPointsData);
    }
    else {
      etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsData;
      do {
        parsedArgument = argument;
        argument = splitString(parsedArgument, ' ');
        calibrationPointsData.insert(String(parsedArgument).toFloat());
      } while(argument != NULL);

      setCalibrationPowerPointsOpen(calibrationPointsData);
    }

    Serial.println("calibration points set");
  }
}

void handleHelp(char* tokens) {
  Serial.println("Use the commands 'help', 'calibrationpoints', 'cleareeprom', 'readinputs', 'calibrationdata', 'calibrateonboot', 'demo', or 'ping'.");
}

void handleClearEeprom(char* tokens) {
  eepromClear();
  Serial.println("Eeprom cleared.");
}

void handleCalibrationData(char* tokens) {
  //dummy load first
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> dummyPowerPoints = calibrationPowerPointsDummy();
  etl::iset<float, std::less<float>>::const_iterator dummyPowerPointsItr = dummyPowerPoints.begin();
  //iterate through dummy load power points
  while (dummyPowerPointsItr != dummyPowerPoints.end())
  {
    float currentPowerPoint = *dummyPowerPointsItr++;
    Serial.print("Calibration data for ");
    Serial.print(String(currentPowerPoint));
    Serial.println("w into a dummy load");

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, true);
    Serial.print("        fwd: ");
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print("        rvr: ");
    Serial.println(String(currentCalibrationData.rvr));
    Serial.print("  magnitude: ");
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print("      phase: ");
    Serial.println(String(currentCalibrationData.phase));
    Serial.print("       vref: ");
    Serial.println(String(currentCalibrationData.vref));
  }

  //open load next
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> openPowerPoints = calibrationPowerPointsOpen();
  etl::iset<float, std::less<float>>::const_iterator openPowerPointsItr = openPowerPoints.begin();
  //iterate through dummy load power points
  while (openPowerPointsItr != openPowerPoints.end())
  {
    float currentPowerPoint = *openPowerPointsItr++;
    Serial.print("Calibration data for ");
    Serial.print(String(currentPowerPoint));
    Serial.println("w into a open load");

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, false);
    Serial.print("        fwd: ");
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print("        rvr: ");
    Serial.println(String(currentCalibrationData.rvr));
    Serial.print("  magnitude: ");
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print("      phase: ");
    Serial.println(String(currentCalibrationData.phase));
    Serial.print("       vref: ");
    Serial.println(String(currentCalibrationData.vref));
  }

}

void handleReadInputs(char* tokens) {
  Serial.print("POWER_FWD_PIN: ");
  Serial.println(String(analogRead(POWER_FWD_PIN)));
  Serial.print("POWER_RVR_PIN: ");
  Serial.println(String(analogRead(POWER_RVR_PIN)));
  Serial.print("COMPLEX_VREF_PIN: ");
  Serial.println(String(analogRead(COMPLEX_VREF_PIN)));
  Serial.print("COMPLEX_PHASE_PIN: ");
  Serial.println(String(analogRead(COMPLEX_PHASE_PIN)));
  Serial.print("COMPLEX_MAGNITUDE_PIN: ");
  Serial.println(String(analogRead(COMPLEX_MAGNITUDE_PIN)));
}

void handleCalibrateOnBoot(char* tokens) {
  char* argument = strtok(NULL, " ");
  if( argument == NULL ) {
    Serial.print("calibrateonboot: ");
    Serial.println((calibrateOnBoot() == true ? "on" : "off"));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals("on") ) {
      activateCalibrateOnBoot();
      Serial.println("Activating calibrateonboot.");
    }
    else if( argumentStr.equals("off") ) {
      deactivateCalibrateOnBoot();
      Serial.println("Deactivating calibrateonboot.");
    }
    else {
      Serial.println("Invalid argument, argument to calibrateonboot command must be either 'on' or 'off'");
    }
  }
}

void handlePing(char* tokens) {
  Serial.println("Pong!");
}

void handleDemo(char* tokens) {
  char* argument = strtok(NULL, " ");
  if( argument == NULL ) {
    Serial.print("demoMode: ");
    Serial.println((demoMode() == true ? "on" : "off"));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals("on") ) {
      activateDemoMode();
      Serial.println("Activating demo.");
    }
    else if( argumentStr.equals("off") ) {
      deactivateDemoMode();
      Serial.println("Deactivating demo.");
    }
    else {
      Serial.println("Invalid argument, argument to demo command must be either 'on' or 'off'");
    }
  }
}
