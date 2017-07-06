#include <math.h>
#include <CommandLine.h>

#include "swr_display.h"

#define POWER_FWD_PIN A15
#define POWER_RVR_PIN A14

#define CALIBRATE_FWD_5W 73
#define CALIBRATE_FWD_200W 689
#define CALIBRATE_RVR_5W 102
#define CALIBRATE_RVR_200W 720

boolean demo_active = false;
boolean demo_power_fwd_increasing = true;
boolean demo_power_rvr_increasing = true;

float power_fwd = 0.0;
float power_rvr = 0.0;
float calibrate_fwd_slope = 0.0;
float calibrate_fwd_intercept = 0.0;
float calibrate_rvr_slope = 0.0;
float calibrate_rvr_intercept = 0.0;

CommandLine commandLine(Serial, "> ");

void setup()   {                
  displaySetup();

  calibrateFwd(5, CALIBRATE_FWD_5W, 200, CALIBRATE_FWD_200W);
  calibrateRvr(5, CALIBRATE_RVR_5W, 200, CALIBRATE_RVR_200W);

  Serial.begin(9600);

  commandLine.add("help", handleHelp);
  commandLine.add("ping", handlePing);
}

void loop() {
  commandLine.update();
  
  if( !demo_active )
    updateInputs();
  render(power_fwd, power_rvr);
  delay(25);
  if(demo_active)
    adjustDemoValues();
}

void handlePing(char* tokens)
{
  Serial.println("Pong!");
}

void handleHelp(char* tokens)
{
  Serial.println("Use the commands 'help' or 'ping'.");
}

float calculateCalibrationSlope(float lowVoltage, uint16_t lowAdc, float highVoltage, uint16_t highAdc) {
  return (highVoltage - lowVoltage) / ((float) (highAdc - lowAdc));
}

float calculateCalibrationIntercept(float slope, float voltage, uint16_t adc) {
  return voltage - slope * ((float)adc);
}

void calibrateFwd(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
  float lowVoltage = powerToVoltage(lowPower);
  float highVoltage = powerToVoltage(highPower);
  calibrate_fwd_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
  calibrate_fwd_intercept = calculateCalibrationIntercept(calibrate_fwd_slope, highVoltage, highAdc);
}

void calibrateRvr(float lowPower, uint16_t lowAdc, float highPower, uint16_t highAdc) {
  float lowVoltage = powerToVoltage(lowPower);
  float highVoltage = powerToVoltage(highPower);
  calibrate_rvr_slope = calculateCalibrationSlope(lowVoltage, lowAdc, highVoltage, highAdc);
  calibrate_rvr_intercept = calculateCalibrationIntercept(calibrate_rvr_slope, highVoltage, highAdc);
}

float calculateFwdPower(uint16_t adcValue) {
  float calculatedVoltage = ((float)adcValue) * calibrate_fwd_slope + calibrate_fwd_intercept;
  return voltageToPower(calculatedVoltage);
}

float calculateRvrPower(uint16_t adcValue) {
  float calculatedVoltage = ((float)adcValue) * calibrate_rvr_slope + calibrate_rvr_intercept;
  return voltageToPower(calculatedVoltage);
}

float voltageToPower(float voltage) {
  return (voltage*voltage) / 50.0;
}

float powerToVoltage(float power) {
  return sqrt(power * 50.0);
}

void updateInputs() {
  updatePowerFwd();
  updatePowerRvr();
}

void updatePowerFwd() {
  uint16_t adcValue = analogRead(POWER_FWD_PIN);
  power_fwd = calculateFwdPower(adcValue);
}

void updatePowerRvr() {
  uint16_t adcValue = analogRead(POWER_RVR_PIN);
  power_rvr = calculateRvrPower(adcValue);
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

