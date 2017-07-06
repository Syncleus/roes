#include <CommandLine.h>

#include "swr_display.h"
#include "swr_power.h"
#include "swr_eeprom.h"

boolean error = false;
String errorMsgLine1 = "";
String errorMsgLine2 = "";
String errorMsgLine3 = "";
String errorMsgLine4 = "";

float power_fwd = 0.0;
float power_rvr = 0.0;

CommandLine commandLine(Serial, "> ");

void setup()   {                
  displaySetup();
  powerSetup();
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

  Serial.begin(9600);

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
  commandLine.add("calibrateonboot", handleCalibrateOnBoot);
}

void loop() {
  unsigned long time = millis();
  
  commandLine.update();

  if( time%25 == 0 && error == false) {
    if(demoMode())
      updatePowerDemo(power_fwd, power_rvr);
    else
      updatePower(power_fwd, power_rvr);
    
    renderSwr(power_fwd, power_rvr);
  }
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

void handleHelp(char* tokens)
{
  Serial.println("Use the commands 'help', 'calibrateonboot', 'demo', or 'ping'.");
}

