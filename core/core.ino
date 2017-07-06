#include <CommandLine.h>

#include "swr_display.h"
#include "swr_power.h"

boolean demo_active = false;
boolean demo_power_fwd_increasing = true;
boolean demo_power_rvr_increasing = true;

float power_fwd = 0.0;
float power_rvr = 0.0;

CommandLine commandLine(Serial, "> ");

void setup()   {                
  displaySetup();
  powerSetup();

  Serial.begin(9600);

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
  commandLine.add("demo", handleDemo);
}

void loop() {
  unsigned long time = millis();
  
  commandLine.update();

  if( time%25 == 0 ) {
    if(demo_active)
      adjustDemoValues();
    else
      updatePower(power_fwd, power_rvr);
    
    render(power_fwd, power_rvr);
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
  Serial.println("Use the commands 'help', 'demo', or 'ping'.");
}

void adjustDemoValues() {
  if( power_fwd < 10.0 )
    if( demo_power_fwd_increasing )
      power_fwd += 0.1;
    else
      power_fwd -= 0.1;
  else if(power_fwd >= 100)
    if( demo_power_fwd_increasing )
      power_fwd += 10.0;
    else
      power_fwd -= 10.0;
  else
    if( demo_power_fwd_increasing )
      power_fwd += 1.0;
    else
      power_fwd -= 1.0;
      
  if(power_fwd > 1000.0) {
    power_fwd = 1000.0;
    demo_power_fwd_increasing = false;
  }
  else if(power_fwd < 0.0 ) {
    power_fwd = 0.0;
    demo_power_fwd_increasing = true;
  }

  if( power_rvr < 10.0 )
    if( demo_power_rvr_increasing )
      power_rvr += 0.2;
    else
      power_rvr -= 0.2;
  else if(power_rvr >= 100)
    if( demo_power_rvr_increasing )
      power_rvr += 20.0;
    else
      power_rvr -= 20.0;
  else
    if( demo_power_rvr_increasing )
      power_rvr += 2.0;
    else
      power_rvr -= 2.0;
  if(power_rvr > power_fwd) {
    power_rvr = power_fwd;
    demo_power_rvr_increasing = false;
  }
  else if( power_rvr < 0.0 ) {
    power_rvr = 0.0;
    demo_power_rvr_increasing = true;
  }
}

