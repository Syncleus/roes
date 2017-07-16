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
boolean calibratingDummy = true;

Screen currentScreen = POWER;

float magnitudeDb = 0.0;
float phase = 0.0;
float power_fwd = 0.0;
float power_rvr = 0.0;

float calibratingPowerPoint = -1.0;

void setup()   {
  Serial.begin(9600);
  heartbeatSetup();
  statusLedSetup();
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
      calibrating = true;
      calibratingPause = false;
      calibratingDummy = true;
      bumpCalibratingPowerPoint();

      renderCalibration(calibratingPowerPoint, calibratingDummy);
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
          if( bumpCalibratingPowerPoint() ) {
            if( calibratingDummy ) {
              calibratingDummy = false;
            }
            else {
              calibrating = false;
              calibratingDummy = true;
              calibratingPause = false;
              deactivateCalibrateOnBoot();
              return;
            }
          }
          calibratingPause = false;
          renderCalibration(calibratingPowerPoint, calibratingDummy);
        }
    }
    else {
      if( runCalibration() ) {
        calibratingPause = true;
        CalibrationAverages result = getCalibration();
        CalibrationData currentCalibration = calibrationData(calibratingPowerPoint, calibratingDummy);
        currentCalibration.fwd = result.adcFwd;
        currentCalibration.rvr = result.adcRvr;
        currentCalibration.vref = result.adcVref;
        currentCalibration.phase = result.adcPhase;
        currentCalibration.magnitude = result.adcMagnitude;
        setCalibrationData(calibratingPowerPoint, calibratingDummy, currentCalibration);
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
    Serial.print("checking: ");
    Serial.println(String(currentPowerPoint));
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
