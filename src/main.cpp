#include "main.h"

#include <Arduino.h>
#include <CommandLine.h>
#include <ArduinoSTL.h>

#include "swr_display.h"
#include "swr_power.h"
#include "swr_eeprom.h"
#include "swr_heartbeat.h"
#include "swr_calibrate.h"
#include "swr_constants.h"
#include "swr_strings.h"
#include "swr_status_led.h"

enum Screen {
  POWER,
  COMPLEX
};

boolean error = false;

boolean calibrating = false;
boolean calibratingPause = false;
boolean calibratingOpen = false;

Screen currentScreen = POWER;

float magnitudeDb = 0.0;
float phase = 0.0;
float power_fwd = 0.0;
float power_rvr = 0.0;

float calibratingPowerPoint = -1.0;

CommandLine commandLine(Serial, "> ");

void setup()   {
  heartbeatSetup();
  statusLedSetup();
  Serial.begin(9600);
  displaySetup();
  eepromSetup();

  pinMode(DOWN_BUTTON_PIN, INPUT);

  //make sure eeprom isn't corrupt
  if( checkEepromCrc() == false ) {
    error = true;
    uint32_t actualCrc = eepromCrc32Actual();
    uint32_t storedCrc = eepromCrc32Stored();
    char errorMsgLine3[11] = "0x";
    uint32toa(actualCrc, errorMsgLine3 + 2, 16);
    char errorMsgLine4[11] = "0x";
    uint32toa(storedCrc, errorMsgLine4 + 2, 16);

    renderError(strings(CORRUPT_EEPROM), strings(CRC_CHECK_FAILED), errorMsgLine3, errorMsgLine4);
  }
  else {
    if( calibrateOnBoot() == true )
    {
      bumpCalibratingPowerPoint();
      calibrating = true;
      calibratingPause = false;
      calibratingOpen = false;

      renderCalibration(calibratingPowerPoint, calibratingOpen);
    }
  }

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
  commandLine.add("calibrationpoints", handleCalibrationPoints);
  commandLine.add("cleareeprom", handleClearEeprom);
  commandLine.add("calibrateonboot", handleCalibrateOnBoot);
  commandLine.add("calibrationdata", handleCalibrationData);
  commandLine.add("readinputs", handleReadInputs);
}

void loop() {
  unsigned long time = millis();

  heartbeatUpdate();
  statusLedUpdate();
  commandLine.update();

  if( error )
    return;

  updateDownButton();

  if( time%25 == 0&& !calibrating) {
    if(demoMode()) {
      updateComplexDemo(&magnitudeDb, &phase);
      updatePowerDemo(&power_fwd, &power_rvr);
    }
    else {
      updateComplex(&magnitudeDb, &phase);
      updatePower(&power_fwd, &power_rvr);
    }

    float swr;
    if( currentScreen == POWER )
      swr = powerToSwr(power_fwd, power_rvr);
    else if( currentScreen == COMPLEX )
      swr = dbToSwr(magnitudeDb);

    if( power_fwd >= 0.1 ) {
      if( swr < 1.5 )
        setLedStatus(SLOW);
      else if( swr >= 1.5 && swr < 2.0 )
        setLedStatus(FAST);
      else if( swr >= 2.0 && swr < 3.0 )
        setLedStatus(VERY_FAST);
      else
        setLedStatus(ON);
    }
    else
      setLedStatus(OFF);

    switch( currentScreen ) {
    case POWER:
      renderPowerSwr(power_fwd, power_rvr);
      break;
    case COMPLEX:
      renderComplexSwr(magnitudeDb, phase);
      break;
    }
  }

  if(calibrating) {
    if( calibratingPause ) {
        if(waitForStop()) {
          if( !calibratingOpen ) {
            if( bumpCalibratingPowerPoint() ) {
              calibratingOpen = true;
            }
          }
          else {
            calibrating = false;
            calibratingOpen = false;
            calibratingPause = false;
            deactivateCalibrateOnBoot();
            return;
          }
          calibratingPause = false;

          if( !calibratingOpen )
            renderCalibration(calibratingPowerPoint, calibratingOpen);
          else
            renderCalibration(lowestPowerPoint(), calibratingOpen);
        }
    }
    else {
      if( runCalibration() ) {
        calibratingPause = true;
        CalibrationAverages result = getCalibration();
        if( !calibratingOpen ) {
          CalibrationData currentCalibration = calibrationDataDummy(calibratingPowerPoint);
          currentCalibration.fwd = result.adcFwd;
          currentCalibration.rvr = result.adcRvr;
          currentCalibration.vref = result.adcVref;
          currentCalibration.phase = result.adcPhase;
          currentCalibration.magnitude = result.adcMagnitude;
          setCalibrationDataDummy(calibratingPowerPoint, currentCalibration);
        }
        else {
          CalibrationData currentCalibration = calibrationDataOpen();
          currentCalibration.fwd = result.adcFwd;
          currentCalibration.rvr = result.adcRvr;
          currentCalibration.vref = result.adcVref;
          currentCalibration.phase = result.adcPhase;
          currentCalibration.magnitude = result.adcMagnitude;
          setCalibrationDataOpen(currentCalibration);
        }
        renderStopTransmitting();
      }
    }
  }
}

void updateDownButton() {
  static boolean downButtonLowLast = false;
  int buttonState = digitalRead(DOWN_BUTTON_PIN);

  if( downButtonLowLast == false && buttonState == LOW ) {
    if( currentScreen == POWER )
      currentScreen = COMPLEX;
    else if( currentScreen == COMPLEX )
      currentScreen = POWER;

    downButtonLowLast = true;
  }

  if(downButtonLowLast && buttonState == HIGH)
    downButtonLowLast = false;
}


boolean bumpCalibratingPowerPoint() {
    etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPointData = calibrationPowerPointsDummy();
    etl::iset<float, std::less<float>>::const_iterator itr = powerPointData.begin();

    // Iterate through the list.
    boolean isNext = false;
    float first = *itr;
    while (itr != powerPointData.end())
    {
      float currentPowerPoint = *itr++;
      if( calibratingPowerPoint < 0.0 || isNext ) {
        calibratingPowerPoint = currentPowerPoint;
        return false;
      }
      else if(calibratingPowerPoint == currentPowerPoint)
        isNext = true;
    }

    if(isNext) {
      calibratingPowerPoint = first;
      return true;
    }

    return false;
}

void handleCalibrationData(char* tokens) {
  Serial.print("calibrationLowFwd: ");
  Serial.println(String(calibrationDataDummy(5.0).fwd));
  Serial.print("calibrationLowRvr: ");
  Serial.println(String(calibrationDataDummy(5.0).rvr));
  Serial.print("calibrationLowVref: ");
  Serial.println(String(calibrationDataDummy(5.0).vref));
  Serial.print("calibrationLowMagnitude: ");
  Serial.println(String(calibrationDataDummy(5.0).magnitude));
  Serial.print("calibrationLowPhase: ");
  Serial.println(String(calibrationDataDummy(5.0).phase));
  Serial.print("calibrationHighFwd: ");
  Serial.println(String(calibrationDataDummy(200.0).fwd));
  Serial.print("calibrationHighRvr: ");
  Serial.println(String(calibrationDataDummy(200.0).rvr));
  Serial.print("calibrationHighVref: ");
  Serial.println(String(calibrationDataDummy(200.0).vref));
  Serial.print("calibrationHighMagnitude: ");
  Serial.println(String(calibrationDataDummy(200.0).magnitude));
  Serial.print("calibrationHighPhase: ");
  Serial.println(String(calibrationDataDummy(200.0).phase));
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

//splits a string in place, no dynamic allocation
char* splitString(char* data, char separator) {
  int dataIndex = -1;
  char currentChar = '\0';
  do {
    dataIndex++;
    currentChar = data[dataIndex];
  } while(currentChar != '\0' && currentChar != separator);

  if( currentChar == '\0' )
    return NULL;

  data[dataIndex] = '\0';
  return data + dataIndex + 1;
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

      setcalibrationPowerPointsDummy(calibrationPointsData);
    }
    else {
      etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> calibrationPointsData;
      do {
        parsedArgument = argument;
        argument = splitString(parsedArgument, ' ');
        calibrationPointsData.insert(String(parsedArgument).toFloat());
      } while(argument != NULL);

      setcalibrationPowerPointsOpen(calibrationPointsData);
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
