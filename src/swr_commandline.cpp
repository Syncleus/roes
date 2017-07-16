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
  Serial.print("calibrationLowFwd: ");
  Serial.println(String(calibrationData(5.0, true).fwd));
  Serial.print("calibrationLowRvr: ");
  Serial.println(String(calibrationData(5.0, true).rvr));
  Serial.print("calibrationLowVref: ");
  Serial.println(String(calibrationData(5.0, true).vref));
  Serial.print("calibrationLowMagnitude: ");
  Serial.println(String(calibrationData(5.0, true).magnitude));
  Serial.print("calibrationLowPhase: ");
  Serial.println(String(calibrationData(5.0, true).phase));
  Serial.print("calibrationHighFwd: ");
  Serial.println(String(calibrationData(200.0, true).fwd));
  Serial.print("calibrationHighRvr: ");
  Serial.println(String(calibrationData(200.0, true).rvr));
  Serial.print("calibrationHighVref: ");
  Serial.println(String(calibrationData(200.0, true).vref));
  Serial.print("calibrationHighMagnitude: ");
  Serial.println(String(calibrationData(200.0, true).magnitude));
  Serial.print("calibrationHighPhase: ");
  Serial.println(String(calibrationData(200.0, true).phase));
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
