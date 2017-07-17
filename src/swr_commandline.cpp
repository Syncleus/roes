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
    Serial.print(strings(CALIBRATIONPOINTS_DUMMY_LABEL));
    while (itr != calibrationPointsDataDummy.end())
    {
      Serial.print(String(*itr++));
      Serial.print(" ");
    }
    Serial.println();

    etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsDataOpen = calibrationPowerPointsOpen();
    // Iterate through the list.
    itr = calibrationPointsDataOpen.begin();
    Serial.print(strings(CALIBRATIONPOINTS_OPEN_LABEL));
    while (itr != calibrationPointsDataOpen.end())
    {
      Serial.print(String(*itr++));
      Serial.print(strings(SINGLE_SPACE));
    }
    Serial.println();
  }
  else {
    char* parsedArgument = argument;
    argument = splitString(parsedArgument, ' ');
    boolean isDummyPoints = true;
    if( String(parsedArgument).equals(strings(OPEN_LABEL)) )
      isDummyPoints = false;

    uint8_t entryCount = 0;
    if( isDummyPoints ) {
      etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> calibrationPointsData;
      do {
        parsedArgument = argument;
        argument = splitString(parsedArgument, ' ');
        if( parsedArgument != NULL ) {
          entryCount++;
          if( entryCount > MAX_CALIBRATION_POWER_POINTS_DUMMY ) {
            Serial.print(strings(CALIBRATIONPOINTS_TOO_MANY));
            Serial.println(String(MAX_CALIBRATION_POWER_POINTS_DUMMY));
            return;
          }
          calibrationPointsData.insert(String(parsedArgument).toFloat());
        }
      } while(argument != NULL);

      if( entryCount <= 0 ) {
        Serial.println(strings(CALIBRATIONPOINTS_TOO_FEW));
        return;
      }
      setCalibrationPowerPointsDummy(calibrationPointsData);
    }
    else {
      etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsData;
      do {
        parsedArgument = argument;
        argument = splitString(parsedArgument, ' ');
        if( parsedArgument != NULL ) {
          entryCount++;
          if( entryCount > MAX_CALIBRATION_POWER_POINTS_OPEN ) {
            Serial.print(strings(CALIBRATIONPOINTS_TOO_MANY));
            Serial.println(String(MAX_CALIBRATION_POWER_POINTS_OPEN));
            return;
          }
          calibrationPointsData.insert(String(parsedArgument).toFloat());
        }
      } while(argument != NULL);

      if( entryCount <= 0 ) {
        Serial.println(strings(CALIBRATIONPOINTS_TOO_FEW));
        return;
      }
      setCalibrationPowerPointsOpen(calibrationPointsData);
    }

    Serial.println(strings(CALIBRATIONPOINTS_SET));
  }
}

void handleHelp(char* tokens) {
  Serial.println(strings(COMMANDS_OVERVIEW_HELP));
}

void handleClearEeprom(char* tokens) {
  eepromClear();
  Serial.println(strings(EEPROM_CLEARED));
}

void handleCalibrationData(char* tokens) {
  //dummy load first
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> dummyPowerPoints = calibrationPowerPointsDummy();
  etl::iset<float, std::less<float>>::const_iterator dummyPowerPointsItr = dummyPowerPoints.begin();
  //iterate through dummy load power points
  while (dummyPowerPointsItr != dummyPowerPoints.end())
  {
    float currentPowerPoint = *dummyPowerPointsItr++;
    Serial.print(strings(CALIBRATIONDATA_HEADER_1));
    Serial.print(String(currentPowerPoint));
    Serial.println(strings(CALIBRATIONDATA_HEADER_2_DUMMY));

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, true);
    Serial.print(strings(CALIBRATIONDATA_FWD));
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print(strings(CALIBRATIONDATA_RVR));
    Serial.println(String(currentCalibrationData.rvr));
    Serial.print(strings(CALIBRATIONDATA_MAGNITUDE));
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print(strings(CALIBRATIONDATA_PHASE));
    Serial.println(String(currentCalibrationData.phase));
    Serial.print(strings(CALIBRATIONDATA_VREF));
    Serial.println(String(currentCalibrationData.vref));
  }

  //open load next
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> openPowerPoints = calibrationPowerPointsOpen();
  etl::iset<float, std::less<float>>::const_iterator openPowerPointsItr = openPowerPoints.begin();
  //iterate through dummy load power points
  while (openPowerPointsItr != openPowerPoints.end())
  {
    float currentPowerPoint = *openPowerPointsItr++;
    Serial.print(strings(CALIBRATIONDATA_HEADER_1));
    Serial.print(String(currentPowerPoint));
    Serial.println(strings(CALIBRATIONDATA_HEADER_2_OPEN));

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, false);
    Serial.print(strings(CALIBRATIONDATA_FWD));
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print(strings(CALIBRATIONDATA_RVR));
    Serial.println(String(currentCalibrationData.rvr));
    Serial.print(strings(CALIBRATIONDATA_MAGNITUDE));
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print(strings(CALIBRATIONDATA_PHASE));
    Serial.println(String(currentCalibrationData.phase));
    Serial.print(strings(CALIBRATIONDATA_VREF));
    Serial.println(String(currentCalibrationData.vref));
  }

}

void handleReadInputs(char* tokens) {
  Serial.print(strings(READINPUTS_FWD));
  Serial.println(String(analogRead(POWER_FWD_PIN)));
  Serial.print(strings(READINPUTS_RVR));
  Serial.println(String(analogRead(POWER_RVR_PIN)));
  Serial.print(strings(READINPUTS_VREF));
  Serial.println(String(analogRead(COMPLEX_VREF_PIN)));
  Serial.print(strings(READINPUTS_PHASE));
  Serial.println(String(analogRead(COMPLEX_PHASE_PIN)));
  Serial.print(strings(READINPUTS_MAGNITUDE));
  Serial.println(String(analogRead(COMPLEX_MAGNITUDE_PIN)));
}

void handleCalibrateOnBoot(char* tokens) {
  char* argument = strtok(NULL, strings(SINGLE_SPACE));
  if( argument == NULL ) {
    Serial.print(strings(CALIBRATEONBOOT_LABEL));
    Serial.println((calibrateOnBoot() == true ? strings(CALIBRATEONBOOT_ON) : strings(CALIBRATEONBOOT_OFF)));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals(strings(CALIBRATEONBOOT_ON)) ) {
      activateCalibrateOnBoot();
      Serial.println(strings(CALIBRATEONBOOT_ACTIVATING));
    }
    else if( argumentStr.equals(strings(CALIBRATEONBOOT_OFF)) ) {
      deactivateCalibrateOnBoot();
      Serial.println(strings(CALIBRATEONBOOT_DEACTIVATING));
    }
    else {
      Serial.println(strings(CALIBRATEONBOOT_INVALID_ARGUMENT));
    }
  }
}

void handlePing(char* tokens) {
  Serial.println(strings(PONG_LABEL));
}

void handleDemo(char* tokens) {
  char* argument = strtok(NULL, strings(SINGLE_SPACE));
  if( argument == NULL ) {
    Serial.print(strings(DEMO_LABEL));
    Serial.println((demoMode() == true ? strings(DEMO_ON) : strings(DEMO_OFF)));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals(strings(DEMO_ON)) ) {
      activateDemoMode();
      Serial.println(strings(DEMO_ACTIVATING));
    }
    else if( argumentStr.equals(strings(DEMO_OFF)) ) {
      deactivateDemoMode();
      Serial.println(strings(DEMO_DEACTIVATING));
    }
    else {
      Serial.println(strings(DEMO_INVALID_ARGUMENT));
    }
  }
}
