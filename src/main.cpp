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

void setup()   {
  heartbeatSetup();
  statusLedSetup();
  Serial.begin(9600);
  displaySetup();
  eepromSetup();
  commandlineSetup();

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


}

void loop() {
  unsigned long time = millis();

  heartbeatUpdate();
  statusLedUpdate();
  commandlineUpdate();

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
