#include <CommandLine.h>
#include <ArduinoSTL.h>

#include "swr_display.h"
#include "swr_power.h"
#include "swr_eeprom.h"
#include "swr_calibrate.h"
#include "swr_constants.h"

boolean error = false;
String errorMsgLine1 = "";
String errorMsgLine2 = "";
String errorMsgLine3 = "";
String errorMsgLine4 = "";

enum CalibratingStage {
  NOT_CALIBRATING,
  LOW_RVR,
  HIGH_RVR,
  LOW_FWD,
  HIGH_FWD
};
boolean calibrationPause = false;

CalibratingStage calibrationStage = NOT_CALIBRATING;

float power_fwd = 0.0;
float power_rvr = 0.0;

CommandLine commandLine(Serial, "> ");

void setup()   {                
  displaySetup();
  eepromSetup();

  //make sure eeprom isn't corrupt
  if( checkEepromCrc() == false ) {
    error = true;
    errorMsgLine1 = "EEPROM is corrupt";
    errorMsgLine2 = "CRC check failed";
    uint32_t dataCrc = persistedDataCrc32();
    uint32_t eepromCrc = eepromCrc32();
    errorMsgLine3 = String(dataCrc);
    errorMsgLine4 = String(eepromCrc);

    renderError(errorMsgLine1, errorMsgLine2, errorMsgLine3, errorMsgLine4);
  }
  else {
    if( calibrateOnBoot() == true )
    {
      calibrationStage = LOW_RVR;
      renderCalibration(LOW_POWER, false);
    }
  }

  Serial.begin(9600);

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
  commandLine.add("bands", handleBands);
  commandLine.add("calibrationpoints", handleCalibrationPoints);
  commandLine.add("cleareeprom", handleClearEeprom);
  commandLine.add("calibrateonboot", handleCalibrateOnBoot);
  commandLine.add("calibrationdata", handleCalibrationData);
  commandLine.add("readinputs", handleReadInputs);
}

void loop() {
  unsigned long time = millis();
  
  commandLine.update();

  if( time%25 == 0 && error == false && calibrationStage == NOT_CALIBRATING) {
    if(demoMode())
      updatePowerDemo(power_fwd, power_rvr);
    else
      updatePower(power_fwd, power_rvr);
    
    renderSwr(power_fwd, power_rvr);
  }
  
  if(calibrationStage != NOT_CALIBRATING) {
    switch(calibrationStage) {
    case LOW_RVR:
      if( calibrationPause ) {
        if(waitForStop(false)) {
          calibrationStage = HIGH_RVR;
          renderCalibration(HIGH_POWER, false);
          calibrationPause = false;
        }
      }
      else if( calibrateLowRvr() ) {
        calibrationPause = true;
        renderStopTransmitting();
      }
      break;
    case HIGH_RVR:
      if( calibrationPause ) {
        if(waitForStop(false)) {
          calibrationStage = LOW_FWD;
          renderCalibration(LOW_POWER, true);
          calibrationPause = false;
        }
      }
      else if( calibrateHighRvr() ) {
        calibrationPause = true;
        renderStopTransmitting();
      }
      break;
    case LOW_FWD:
      if( calibrationPause ) {
        if(waitForStop(true)) {
          calibrationStage = HIGH_FWD;
          renderCalibration(HIGH_POWER, true);
          calibrationPause = false;
        }
      }
      else if( calibrateLowFwd() ) {
        calibrationPause = true;
        renderStopTransmitting();
      }
      break;
    case HIGH_FWD:
      if( calibrationPause ) {
        if(waitForStop(true)) {
          Calibration calibration = getCalibration();
          setCalibrationLowFwd(calibration.lowFwd);
          setCalibrationLowRvr(calibration.lowRvr);
          setCalibrationLowVref(calibration.lowVref);
          setCalibrationLowPhase(calibration.lowPhase);
          setCalibrationLowMagnitude(calibration.lowMagnitude);
          setCalibrationLowRatio(calibration.lowRatio);
          setCalibrationHighFwd(calibration.highFwd);
          setCalibrationHighRvr(calibration.highRvr);
          setCalibrationHighVref(calibration.highVref);
          setCalibrationHighPhase(calibration.highPhase);
          setCalibrationHighMagnitude(calibration.highMagnitude);
          setCalibrationHighRatio(calibration.highRatio);
          calibrationStage = NOT_CALIBRATING;
          calibrationPause = false;
          deactivateCalibrateOnBoot();
        }
      }
      else if( calibrateHighFwd() ) {
        calibrationPause = true;
        renderStopTransmitting();
      }
      break;
    default:
      error = true;
      errorMsgLine1 = "Unexpected behavior";
    }
  }
}

void handleCalibrationData(char* tokens)
{
  Serial.print("calibrationLowFwd: ");
  Serial.println(String(calibrationLowFwd()));
  Serial.print("calibrationLowRvr: ");
  Serial.println(String(calibrationLowRvr()));
  Serial.print("calibrationLowVref: ");
  Serial.println(String(calibrationLowVref()));
  Serial.print("calibrationLowMagnitude: ");
  Serial.println(String(calibrationLowMagnitude()));
  Serial.print("calibrationLowPhase: ");
  Serial.println(String(calibrationLowPhase()));
  Serial.print("calibrationLowRatio: ");
  Serial.println(String(calibrationLowRatio()));
  Serial.print("calibrationHighFwd: ");
  Serial.println(String(calibrationHighFwd()));
  Serial.print("calibrationHighRvr: ");
  Serial.println(String(calibrationHighRvr()));
  Serial.print("calibrationHighVref: ");
  Serial.println(String(calibrationHighVref()));
  Serial.print("calibrationHighMagnitude: ");
  Serial.println(String(calibrationHighMagnitude()));
  Serial.print("calibrationHighPhase: ");
  Serial.println(String(calibrationHighPhase()));
  Serial.print("calibrationHighRatio: ");
  Serial.println(String(calibrationHighRatio()));
}

void handleReadInputs(char* tokens)
{
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

void handleCalibrateOnBoot(char* tokens)
{
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

void handlePing(char* tokens)
{
  Serial.println("Pong!");
}

void handleDemo(char* tokens)
{
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
char* splitString(char* data, char separator)
{
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

void handleBands(char* tokens)
{
  char* argument = strtok(NULL, "\0");
  if( argument == NULL ) {
    etl::set<String, MAX_BANDS_COUNT> bandData = bands();
    etl::iset<String, std::less<String>>::const_iterator itr;

    // Iterate through the list.
    itr = bandData.begin();
    Serial.print("bands: ");
    while (itr != bandData.end())
    {
      Serial.print(*itr++);
      Serial.print(" ");
    }
    Serial.println();
  }
  else {
    String argumentStr = String(argument);
    char* argumentCharArray = argumentStr.c_str();

    etl::set<String, MAX_BANDS_COUNT> bandData;
    do {
      char* parsedArgument = argumentCharArray;
      argumentCharArray = splitString(parsedArgument, ' ');
      bandData.insert(String(parsedArgument));
    } while(argumentCharArray != NULL);

    setBands(bandData);
    Serial.println("bands set");
  }
}

void handleCalibrationPoints(char* tokens)
{
  char* argument = strtok(NULL, "\0");
  if( argument == NULL ) {
    etl::set<float, MAX_CALIBRATION_POWER_POINTS> calibrationPointsData = calibrationPowerPoints();
    etl::iset<float, std::less<float>>::const_iterator itr;

    // Iterate through the list.
    itr = calibrationPointsData.begin();
    Serial.print("calibration points: ");
    while (itr != calibrationPointsData.end())
    {
      Serial.print(String(*itr++));
      Serial.print(" ");
    }
    Serial.println();
  }
  else {
    String argumentStr = String(argument);
    char* argumentCharArray = argumentStr.c_str();

    etl::set<float, MAX_CALIBRATION_POWER_POINTS> calibrationPointsData;
    do {
      char* parsedArgument = argumentCharArray;
      argumentCharArray = splitString(parsedArgument, ' ');
      calibrationPointsData.insert(String(parsedArgument).toFloat());
    } while(argumentCharArray != NULL);

    setCalibrationPowerPoints(calibrationPointsData);
    Serial.println("calibration points set");
  }
}

void handleHelp(char* tokens)
{
  Serial.println("Use the commands 'help', 'bands', 'cleareeprom', 'readinputs', 'calibrationdata', 'calibrateonboot', 'demo', or 'ping'.");
}

void handleClearEeprom(char* tokens)
{
  eepromClear();
  Serial.println("Eeprom cleared.");
}

