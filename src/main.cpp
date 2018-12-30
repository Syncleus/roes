#include "main.h"

#include "swr_display.h"
#include "swr_power.h"
#include "swr_heartbeat.h"
#include "swr_calibrate.h"
#include "swr_constants.h"
#include "swr_strings.h"
#include "swr_status_led.h"
#include "swr_commandline.h"
#include "swr_eeprom.h"
#include "swr_sensors.h"

enum TopScreen {
  TOP_POWER,
  TOP_REFLECTION
};

enum BottomScreen {
  BOTTOM_POWER,
  BOTTOM_REFLECTION,
  BOTTOM_LOAD
};

boolean error = false;

boolean calibrating = false;
boolean calibratingPause = false;
boolean calibratingDummy = true;

TopScreen currentTopScreen = TOP_POWER;
BottomScreen currentBottomScreen = BOTTOM_POWER;

SensorData sensorData;

float calibratingPowerPoint = -1.0;

void setup()   {
  Serial.begin(SERIAL_BAUDE_RATE);//SERIAL_BAUDE_RATE);
  while (!Serial);     // used for leonardo debugging

  Serial.println("Begining system initialization...");
  heartbeatSetup();
  Serial.println("  Heartbeat initialized");
  statusLedSetup();
  Serial.println("  Status LED initialized");
  displaySetup();
  Serial.println("  Display initialized");
  eepromSetup();
  Serial.println("  EEPROM initialized");
  commandlineSetup();
  Serial.println("  Serial Commandline initialized");
  Serial.println("System initialized!");

  pinMode(DOWN_BUTTON_PIN, INPUT);
  pinMode(UP_BUTTON_PIN, INPUT);

  Serial.println("Checking EEPROM integrity...");
  //make sure eeprom isn't corrupt
  if( checkEepromCrc() == false ) {
    Serial.println("EEPROM failed CRC check!");
    error = true;
    uint32_t actualCrc = eepromCrc32Actual();
    uint32_t storedCrc = eepromCrc32Stored();
    char errorMsgLine3[11] = "0x";
    uint32toa(actualCrc, errorMsgLine3 + 2, 16);
    char errorMsgLine4[11] = "0x";
    uint32toa(storedCrc, errorMsgLine4 + 2, 16);

    prepareRender();
    renderError(strings(CORRUPT_EEPROM), strings(CRC_CHECK_FAILED), errorMsgLine3, errorMsgLine4);
    finishRender();
  }
  else {
    Serial.println("EEPROM passed all checks!");

    if( calibrateOnBoot() == true )
    {
      Serial.println("Configured to calibrate on boot...");
      calibrating = true;
      calibratingPause = false;
      calibratingDummy = true;
      bumpCalibratingPowerPoint();

      prepareRender();
      renderCalibration(calibratingPowerPoint, calibratingDummy);
      finishRender();
    }
  }
}

void loop() {
  unsigned long time = millis();
  static unsigned long refreshDisplayTime = 0;

  heartbeatUpdate();
  statusLedUpdate();
  commandlineUpdate();

  if( error )
    return;

  updateDownButton();
  updateUpButton();

  if( (long)(time - refreshDisplayTime) >= 0 && !calibrating) {
    refreshDisplayTime = time + DISPLAY_REFRESH_RATE_MS;

    if(demoMode()) {
      updateComplexDemo(&(sensorData.differentialMagnitudeDb), &(sensorData.differentialPhaseDeg));
      updatePowerDemo(&(sensorData.fwdPower), &(sensorData.reflPower));
      sensorData.swr = powerToSwr(sensorData.fwdPower, sensorData.reflPower);
    }
    else {
      sensorData = readSensors(sensorData);
    }

    // if( sensorData.fwdPower >= TRANSMIT_THREASHOLD_POWER ) {
    //   if( sensorData.swr < 1.5 )
    //     setLedStatus(SLOW);
    //   else if( sensorData.swr >= 1.5 && sensorData.swr < 2.0 )
    //     setLedStatus(FAST);
    //   else if( sensorData.swr >= 2.0 && sensorData.swr < 3.0 )
    //     setLedStatus(VERY_FAST);
    //   else
    //     setLedStatus(ON);
    // }
    // else
    //   setLedStatus(OFF);

    prepareRender();
    renderSwr(sensorData.swr);
    // switch( currentTopScreen ) {
    // case TOP_POWER:
       renderPowerBars(sensorData.fwdPower, sensorData.reflPower);
    // break;
    // case TOP_REFLECTION:
    //   renderReflectionBars(sensorData.differentialMagnitudeDb, sensorData.calculatedPhaseDeg);
    //   break;
    // }
    // switch( currentBottomScreen ) {
    // case BOTTOM_POWER:
       renderPowerText(sensorData.fwdPower, sensorData.reflPower);
    //   break;
    // case BOTTOM_REFLECTION:
       renderReflectionText(sensorData.differentialMagnitudeDb, sensorData.calculatedPhaseDeg);
    //   break;
    // case BOTTOM_LOAD:
       renderLoadZText(sensorData.differentialMagnitudeDb, sensorData.calculatedPhaseDeg);
    //   break;
    // }
    finishRender();
  }

  // if(calibrating) {
  //   if( calibratingPause ) {
  //       if(waitForStop()) {
  //         if( bumpCalibratingPowerPoint() ) {
  //           if( calibratingDummy ) {
  //             calibratingDummy = false;
  //           }
  //           else {
  //             calibrating = false;
  //             calibratingDummy = true;
  //             calibratingPause = false;
  //             deactivateCalibrateOnBoot();
  //             return;
  //           }
  //         }
  //         calibratingPause = false;
  //         prepareRender();
  //         renderCalibration(calibratingPowerPoint, calibratingDummy);
  //         finishRender();
  //       }
  //   }
  //   else {
  //     if( runCalibration() ) {
  //       calibratingPause = true;
  //       CalibrationAverages result = getCalibration();
  //       CalibrationData currentCalibration = calibrationData(calibratingPowerPoint, calibratingDummy);
  //       currentCalibration.fwd = result.adcFwd;
  //       currentCalibration.refl = result.adcRvr;
  //       currentCalibration.vref = result.adcVref;
  //       currentCalibration.phase = result.adcPhase;
  //       currentCalibration.magnitude = result.adcMagnitude;
  //       currentCalibration.phaseShifted = result.adcPhaseShifted;
  //       currentCalibration.vrefShifted = result.adcVrefShifted;
  //       setCalibrationData(calibratingPowerPoint, calibratingDummy, currentCalibration);
  //       prepareRender();
  //       renderStopTransmitting();
  //       finishRender();
  //     }
  //   }
  // }
}

void updateDownButton() {
  static boolean downButtonLowLast = false;
  int buttonState = digitalRead(DOWN_BUTTON_PIN);

  if( downButtonLowLast == false && buttonState == LOW ) {
    if( currentBottomScreen == BOTTOM_POWER )
      currentBottomScreen = BOTTOM_REFLECTION;
    else if( currentBottomScreen == BOTTOM_REFLECTION )
      currentBottomScreen = BOTTOM_LOAD;
    else if( currentBottomScreen == BOTTOM_LOAD )
      currentBottomScreen = BOTTOM_POWER;

    downButtonLowLast = true;
  }

  if(downButtonLowLast && buttonState == HIGH)
    downButtonLowLast = false;
}

void updateUpButton() {
  static boolean upButtonLowLast = false;
  int buttonState = digitalRead(UP_BUTTON_PIN);

  if( upButtonLowLast == false && buttonState == LOW ) {
    if( currentTopScreen == TOP_POWER )
      currentTopScreen = TOP_REFLECTION;
    else if( currentTopScreen == TOP_REFLECTION )
      currentTopScreen = TOP_POWER;

    upButtonLowLast = true;
  }

  if(upButtonLowLast && buttonState == HIGH)
    upButtonLowLast = false;
}


boolean bumpCalibratingPowerPoint() {
  etl::iset<float, std::less<float>>::const_iterator itr;
  etl::iset<float, std::less<float>>::const_iterator itrEnd;

  etl::set<float, MAX_CALIBRATION_POWER_POINTS_DUMMY> powerPointDataDummy = calibrationPowerPointsDummy();
  etl::set<float, MAX_CALIBRATION_POWER_POINTS_OPEN> powerPointDataOpen = calibrationPowerPointsOpen();
  if(calibratingDummy) {
    itr = powerPointDataDummy.begin();
    itrEnd = powerPointDataDummy.end();
  }
  else {
    itr = powerPointDataOpen.begin();
    itrEnd = powerPointDataOpen.end();
  }

  // Iterate through the list.
  boolean isNext = false;
  while (itr != itrEnd)
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
    if( calibratingDummy )
      calibratingPowerPoint = *(calibrationPowerPointsOpen().begin());
    else
      calibratingPowerPoint = *(calibrationPowerPointsDummy().begin());
    return true;
  }

  return false;
}
