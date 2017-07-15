#include <CommandLine.h>
#include <ArduinoSTL.h>

#include "swr_display.h"
#include "swr_power.h"
#include "swr_eeprom.h"
#include "swr_heartbeat.h"
#include "swr_calibrate.h"
#include "swr_constants.h"

enum Screen {
  POWER,
  COMPLEX
};

boolean error = false;
String errorMsgLine1 = "";
String errorMsgLine2 = "";
String errorMsgLine3 = "";
String errorMsgLine4 = "";

boolean calibrating = false;
boolean calibratingPause = false;
boolean calibratingFwd = false;

Screen currentScreen = POWER;

float magnitudeDb = 0.0;
float phase = 0.0;
float power_fwd = 0.0;
float power_rvr = 0.0;

boolean downButtonLowLast = false;

String calibratingBand = "";
float calibratingPowerPoint = -1.0;

CommandLine commandLine(Serial, "> ");

void setup()   {  
  heartbeatSetup();
  Serial.begin(9600);              
  displaySetup();
  eepromSetup();

  pinMode(DOWN_BUTTON_PIN, INPUT);

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
      bumpCalibratingBand();
      Serial.print("calibrating band: ");
      Serial.println(calibratingBand);
      bumpCalibratingPowerPoint();
      calibrating = true;
      calibratingPause = false;
      calibratingFwd = false;

      renderCalibration(calibratingPowerPoint, calibratingFwd, calibratingBand);
    }
  }

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

  heartbeatUpdate();
  commandLine.update();

  updateScreenFromButton();

  if( time%25 == 0 && error == false && !calibrating) {
    if(demoMode()) {
      updateComplexDemo(&magnitudeDb, &phase);
      updatePowerDemo(&power_fwd, &power_rvr);
    }
    else {
      updateComplex(&magnitudeDb, &phase);
      updatePower(&power_fwd, &power_rvr);
    }

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
        if(waitForStop(calibratingFwd)) {
          if( bumpCalibratingPowerPoint() ) {
            if( bumpCalibratingBand() ) {
              if( calibratingFwd == false ) {
                calibratingFwd = true;
              }
              else {
                calibrating = false;
                calibratingFwd = false;
                calibratingPause = false;
                deactivateCalibrateOnBoot();
                return;
              }
            }
          }
          calibratingPause = false;
          renderCalibration(calibratingPowerPoint, calibratingFwd, calibratingBand);
        }
    }
    else {
      if( runCalibration() ) {
        calibratingPause = true;
        CalibrationAverages result = getCalibration();
        CalibrationData currentCalibration = calibrationData(calibratingBand.c_str(), calibratingPowerPoint);
        if( calibratingFwd ) {
          currentCalibration.fwd = result.adcFwd;
          currentCalibration.fwdRefl = result.adcRvr;
          currentCalibration.vref = result.adcVref;
          currentCalibration.phase = result.adcPhase;
          currentCalibration.magnitude = result.adcMagnitude;
        }
        else {
          currentCalibration.rvr = result.adcRvr;
        }
        setCalibrationData(calibratingBand.c_str(), calibratingPowerPoint, currentCalibration);
        renderStopTransmitting();
      }
    }
  }
}

void updateScreenFromButton() {
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

boolean bumpCalibratingBand() {
    etl::set<String, MAX_BANDS_COUNT> bandData = bands();
    etl::iset<String, std::less<String>>::const_iterator itr = bandData.begin();

    // Iterate through the list.
    boolean isNext = false;
    String firstBand = *itr;
    while (itr != bandData.end())
    {
      String currentBand = *itr++;
      if( calibratingBand.equals("") || isNext ) {
         calibratingBand = currentBand;
        return false;
      }
      else if(calibratingBand.equals(currentBand))
        isNext = true;
    }

    if(isNext) {
      calibratingBand = firstBand;
      return true;
    }
}

boolean bumpCalibratingPowerPoint() {
    etl::set<float, MAX_CALIBRATION_POWER_POINTS> powerPointData = calibrationPowerPoints();
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
}

void handleCalibrationData(char* tokens)
{
  Serial.print("calibrationLowFwd: ");
  Serial.println(String(calibrationData("15m", 5.0).fwd));
  Serial.print("calibrationLowRvr: ");
  Serial.println(String(calibrationData("15m", 5.0).rvr));
  Serial.print("calibrationLowVref: ");
  Serial.println(String(calibrationData("15m", 5.0).vref));
  Serial.print("calibrationLowMagnitude: ");
  Serial.println(String(calibrationData("15m", 5.0).magnitude));
  Serial.print("calibrationLowPhase: ");
  Serial.println(String(calibrationData("15m", 5.0).phase));
  Serial.print("calibrationLowRefl: ");
  Serial.println(String(calibrationData("15m", 5.0).fwdRefl));
  Serial.print("calibrationHighFwd: ");
  Serial.println(String(calibrationData("15m", 200.0).fwd));
  Serial.print("calibrationHighRvr: ");
  Serial.println(String(calibrationData("15m", 200.0).rvr));
  Serial.print("calibrationHighVref: ");
  Serial.println(String(calibrationData("15m", 200.0).vref));
  Serial.print("calibrationHighMagnitude: ");
  Serial.println(String(calibrationData("15m", 200.0).magnitude));
  Serial.print("calibrationHighPhase: ");
  Serial.println(String(calibrationData("15m", 200.0).phase));
  Serial.print("calibrationHighRefl: ");
  Serial.println(String(calibrationData("15m", 200.0).fwdRefl));
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
  Serial.println("Use the commands 'help', 'bands', 'calibrationpoints', 'cleareeprom', 'readinputs', 'calibrationdata', 'calibrateonboot', 'demo', or 'ping'.");
}

void handleClearEeprom(char* tokens)
{
  eepromClear();
  Serial.println("Eeprom cleared.");
}

