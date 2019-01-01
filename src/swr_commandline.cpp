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
  commandLine.add("swrsource", handleSwrSource);
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
    Serial.print(CALIBRATIONPOINTS_DUMMY_LABEL);
    while (itr != calibrationPointsDataDummy.end())
    {
      Serial.print(String(*itr++));
      Serial.print(" ");
    }
    Serial.println();

    etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsDataOpen = calibrationPowerPointsOpen();
    // Iterate through the list.
    itr = calibrationPointsDataOpen.begin();
    Serial.print(CALIBRATIONPOINTS_OPEN_LABEL);
    while (itr != calibrationPointsDataOpen.end())
    {
      Serial.print(String(*itr++));
      Serial.print(ONE_SPACE);
    }
    Serial.println();
  }
  else {
    char* parsedArgument = argument;
    argument = splitString(parsedArgument, ' ');
    boolean isDummyPoints = true;
    if( String(parsedArgument).equals(OPEN_LABEL) )
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
            Serial.print(CALIBRATIONPOINTS_TOO_MANY);
            Serial.println(String(MAX_CALIBRATION_POWER_POINTS_DUMMY));
            return;
          }
          calibrationPointsData.insert(String(parsedArgument).toFloat());
        }
      } while(argument != NULL);

      if( entryCount <= 0 ) {
        Serial.println(CALIBRATIONPOINTS_TOO_FEW);
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
            Serial.print(CALIBRATIONPOINTS_TOO_MANY);
            Serial.println(String(MAX_CALIBRATION_POWER_POINTS_OPEN));
            return;
          }
          calibrationPointsData.insert(String(parsedArgument).toFloat());
        }
      } while(argument != NULL);

      if( entryCount <= 0 ) {
        Serial.println(CALIBRATIONPOINTS_TOO_FEW);
        return;
      }
      setCalibrationPowerPointsOpen(calibrationPointsData);
    }

    Serial.println(CALIBRATIONPOINTS_SET);
  }
}

void handleHelp(char* tokens) {
  Serial.println(COMMANDS_OVERVIEW_HELP);
  Serial.println(HELP_COMMANDS_1);
  Serial.println(HELP_COMMANDS_2);
  Serial.println(HELP_COMMANDS_3);
  Serial.println(HELP_COMMANDS_4);
  Serial.println(HELP_COMMANDS_5);
  Serial.println(HELP_COMMANDS_6);
  Serial.println(HELP_COMMANDS_7);
  Serial.println(HELP_COMMANDS_8);
  Serial.println(HELP_COMMANDS_9);
}

void handleClearEeprom(char* tokens) {
  eepromClear();
  Serial.println(EEPROM_CLEARED);
}

void handleCalibrationData(char* tokens) {
  //dummy load first
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> dummyPowerPoints = calibrationPowerPointsDummy();
  etl::iset<float, std::less<float>>::const_iterator dummyPowerPointsItr = dummyPowerPoints.begin();
  //iterate through dummy load power points
  while (dummyPowerPointsItr != dummyPowerPoints.end())
  {
    float currentPowerPoint = *dummyPowerPointsItr++;
    Serial.print(CALIBRATIONDATA_HEADER_1);
    Serial.print(String(currentPowerPoint));
    Serial.println(CALIBRATIONDATA_HEADER_2_DUMMY);

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, true);
    Serial.print(CALIBRATIONDATA_FWD);
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print(CALIBRATIONDATA_RVR);
    Serial.println(String(currentCalibrationData.refl));
    Serial.print(CALIBRATIONDATA_MAGNITUDE);
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print(CALIBRATIONDATA_PHASE);
    Serial.println(String(currentCalibrationData.phase));
    Serial.print(CALIBRATIONDATA_VREF);
    Serial.println(String(currentCalibrationData.vref));
    Serial.print(CALIBRATIONDATA_VREF_SHIFTED);
    Serial.println(String(currentCalibrationData.vrefShifted));
    Serial.print(CALIBRATIONDATA_PHASE_SHIFTED);
    Serial.println(String(currentCalibrationData.phaseShifted));
  }

  //open load next
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> openPowerPoints = calibrationPowerPointsOpen();
  etl::iset<float, std::less<float>>::const_iterator openPowerPointsItr = openPowerPoints.begin();
  //iterate through dummy load power points
  while (openPowerPointsItr != openPowerPoints.end())
  {
    float currentPowerPoint = *openPowerPointsItr++;
    Serial.print(CALIBRATIONDATA_HEADER_1);
    Serial.print(String(currentPowerPoint));
    Serial.println(CALIBRATIONDATA_HEADER_2_OPEN);

    CalibrationData currentCalibrationData = calibrationData(currentPowerPoint, false);
    Serial.print(CALIBRATIONDATA_FWD);
    Serial.println(String(currentCalibrationData.fwd));
    Serial.print(CALIBRATIONDATA_RVR);
    Serial.println(String(currentCalibrationData.refl));
    Serial.print(CALIBRATIONDATA_MAGNITUDE);
    Serial.println(String(currentCalibrationData.magnitude));
    Serial.print(CALIBRATIONDATA_PHASE);
    Serial.println(String(currentCalibrationData.phase));
    Serial.print(CALIBRATIONDATA_VREF);
    Serial.println(String(currentCalibrationData.vref));
    Serial.print(CALIBRATIONDATA_VREF_SHIFTED);
    Serial.println(String(currentCalibrationData.vrefShifted));
    Serial.print(CALIBRATIONDATA_PHASE_SHIFTED);
    Serial.println(String(currentCalibrationData.phaseShifted));
  }

}

void handleReadInputs(char* tokens) {
  Serial.print(READINPUTS_FWD);
  Serial.println(String(analogRead(POWER_FWD_PIN)));
  Serial.print(READINPUTS_RVR);
  Serial.println(String(analogRead(POWER_REFL_PIN)));
  Serial.print(READINPUTS_VREF);
  Serial.println(String(analogRead(DIFFERENTIAL_VREF_PIN)));
  Serial.print(READINPUTS_PHASE);
  Serial.println(String(analogRead(DIFFERENTIAL_PHASE_PIN)));
  Serial.print(READINPUTS_MAGNITUDE);
  Serial.println(String(analogRead(DIFFERENTIAL_MAGNITUDE_PIN)));
}

void handleCalibrateOnBoot(char* tokens) {
  char* argument = strtok(NULL, ONE_SPACE);
  if( argument == NULL ) {
    Serial.print(CALIBRATEONBOOT_LABEL);
    Serial.println((calibrateOnBoot() == true ? CALIBRATEONBOOT_ON : CALIBRATEONBOOT_OFF));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals(CALIBRATEONBOOT_ON) ) {
      activateCalibrateOnBoot();
      Serial.println(CALIBRATEONBOOT_ACTIVATING);
    }
    else if( argumentStr.equals(CALIBRATEONBOOT_OFF) ) {
      deactivateCalibrateOnBoot();
      Serial.println(CALIBRATEONBOOT_DEACTIVATING);
    }
    else {
      Serial.println(CALIBRATEONBOOT_INVALID_ARGUMENT);
    }
  }
}

void handlePing(char* tokens) {
  Serial.println(PONG_LABEL);
}

void handleDemo(char* tokens) {
  char* argument = strtok(NULL, ONE_SPACE);
  if( argument == NULL ) {
    Serial.print(DEMO_LABEL);
    Serial.println((demoMode() ? DEMO_ON : DEMO_OFF));
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals(DEMO_ON) ) {
      activateDemoMode();
      Serial.println(DEMO_ACTIVATING);
    }
    else if( argumentStr.equals(DEMO_OFF) ) {
      deactivateDemoMode();
      Serial.println(DEMO_DEACTIVATING);
    }
    else {
      Serial.println(DEMO_INVALID_ARGUMENT);
    }
  }
}

void handleSwrSource(char* tokens) {
  char* argument = strtok(NULL, ONE_SPACE);
  if( argument == NULL ) {
    Serial.print(SWR_SOURCE_INFO);
    if( differentialForSwr() )
      Serial.println(SWR_SOURCE_DIFFERENTIAL);
    else if( envelopeDetectorForSwr() )
      Serial.println(SWR_SOURCE_ENVELOPE);
  }
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals(SWR_SOURCE_DIFFERENTIAL) ) {
      activateAd8302ForSwr();
      Serial.println(SWR_SOURCE_DIFFERENTIAL_SET);
    }
    else if( argumentStr.equals(SWR_SOURCE_ENVELOPE) ) {
      activeEnvelopeDetectorForSwr();
      Serial.println(SWR_SOURCE_ENVELOPE_SET);
    }
    else {
      Serial.println(SWR_SOURCE_INVALID);
    }
  }
}
