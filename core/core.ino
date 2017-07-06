#include <CommandLine.h>

#include "swr_display.h"
#include "swr_power.h"
#include "swr_eeprom.h"

boolean demo_active = false;

float power_fwd = 0.0;
float power_rvr = 0.0;

CommandLine commandLine(Serial, "> ");

void setup()   {                
  displaySetup();
  powerSetup();
  eepromSetup();

  Serial.begin(9600);

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
  commandLine.add("calibrateonboot", handleCalibrateOnBoot);
}

void loop() {
  unsigned long time = millis();
  
  commandLine.update();

  if( time%25 == 0 ) {
    if(demo_active)
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
      resetCalibrateOnBoot();
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
  if( argument == NULL )
    Serial.println("demo command must have an argument of either 'on' or 'off'");
  else {
    String argumentStr = String(argument);
    if( argumentStr.equals("on") ) {
      demo_active = true;
      Serial.println("Activating demo.");
    }
    else if( argumentStr.equals("off") ) {
      demo_active = false;
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

