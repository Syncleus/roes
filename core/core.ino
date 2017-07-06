#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define SCREEN_HEIGHT SSD1306_LCDHEIGHT
#define SCREEN_WIDTH SSD1306_LCDWIDTH
#define PERCENT_BAR_TITLE_WIDTH 20
#define PERCENT_BAR_WIDTH (SCREEN_WIDTH-PERCENT_BAR_TITLE_WIDTH)
#define CHARACTER_WIDTH 6
#define SCREEN_ROW_1_Y 0
#define SCREEN_ROW_2_Y 16
#define SCREEN_ROW_3_Y 33
#define SCREEN_ROW_4_Y 49
#define SCREEN_ROW_5_Y 57

#define POWER_FWD_PIN A15
#define POWER_RVR_PIN A14

#define CALIBRATE_FWD_5W 73
#define CALIBRATE_FWD_200W 689
#define CALIBRATE_RVR_5W 102
#define CALIBRATE_RVR_200W 720

static const unsigned char PROGMEM gamma16_glcd_bmp[] =
{ B01111111, B11111110,
  B00111111, B11111110,
  B00011000, B00000110,
  B00011000, B00000010,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00011000, B00000000,
  B00111100, B00000000,
  B01111110, B00000000 };

static const unsigned char PROGMEM angle8_glcd_bmp[] =
{ B00000000,
  B00000010,
  B00000100,
  B00001000,
  B00010000,
  B00100000,
  B01111110,
  B00000000 };

boolean demo_active = false;
boolean demo_power_fwd_increasing = true;
boolean demo_power_rvr_increasing = true;

float power_fwd = 0.0;
float power_rvr = 0.0;
float calibrate_fwd_slope = 0.0;
float calibrate_fwd_intercept = 0.0;
float calibrate_rvr_slope = 0.0;
float calibrate_rvr_intercept = 0.0;

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  calibrateFwd(5, CALIBRATE_FWD_5W, 200, CALIBRATE_FWD_200W);
  calibrateRvr(5, CALIBRATE_RVR_5W, 200, CALIBRATE_RVR_200W);
  // init done
}

void loop() {
  if( !demo_active )
    updateInputs();
  render(power_fwd, power_rvr);
  delay(25);
  if(demo_active)
    adjustTestValues();
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

void adjustTestValues() {
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

uint8_t percentBar(uint8_t y_offset, float percent) {
  display.fillRect(PERCENT_BAR_TITLE_WIDTH, y_offset, PERCENT_BAR_WIDTH * percent, 15, 1);
  return PERCENT_BAR_TITLE_WIDTH + (PERCENT_BAR_WIDTH * percent);
}

float logBased(float value, float base) {
  return log10(value)/log10(base);
}

float scaleToPercent(float value, float middle, float scale) {
  if( value == 0 )
    return 0.0;
  else if(isinf(value))
    return 1.0;
  float p = logBased(value / middle, scale);
  float pabs = fabs(p);
  float z = (pow(2.0, pabs) - 1.0) / pow(2.0, pabs);
  if( p < 0 )
    z = -1.0 * z;
  return (z + 1.0) / 2.0;
}

float scaleToPercent(float value, float value_min, float value_mid, float scale) {
  if(isinf(value))
    return 1.0;
  return scaleToPercent(value - value_min, value_mid - value_min, scale);
}

String makeValueLabel(float value, String units) {
  String label = String(value);
  if( isinf(value) )
    return "***";
  int8_t decimalIndex = label.indexOf(".");
  if( decimalIndex > 1 )
    label = label.substring(0,decimalIndex);
  else
    label = label.substring(0, 3);
  label.concat(units);
  return label;
}

float swrFromPower(float power_fwd, float power_rvr) {
  if( power_rvr <= 0 )
    return 1.0;
  float pwrs = sqrt(power_rvr / power_fwd);
  return (1.0 + pwrs)/(1.0 - pwrs);
}

void renderCompleteBar(int8_t y_offset, String label, float value, String units, float value_min, float value_mid, float scale) {
  display.setCursor(0, y_offset + 4);
  display.println(label);
  float barPercent = scaleToPercent(value, value_min, value_mid, scale);
  uint8_t barEnd = percentBar(y_offset, barPercent);
  String valueLabel = makeValueLabel(value, units);
  uint8_t valueLabelWidth = valueLabel.length() * CHARACTER_WIDTH + 2;
  if( barEnd < PERCENT_BAR_TITLE_WIDTH + valueLabelWidth) { 
    display.setCursor(barEnd + 2, y_offset + 4);
    display.println(valueLabel);
  }
  else {
    display.setTextColor(BLACK);
    display.setCursor(barEnd - valueLabelWidth, y_offset + 4);
    display.println(valueLabel);
    display.setTextColor(WHITE);
  }
}

void render(float power_fwd, float power_rvr) {
  display.clearDisplay();

  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_1_Y, "SWR", (power_rvr <= power_fwd ? swrFromPower(power_fwd, power_rvr) : swrFromPower(power_fwd, power_fwd)), "", 1.0, 2.0, 2.0);
  renderCompleteBar(SCREEN_ROW_2_Y, "Fwd", power_fwd, "w", 0.0, 100.0, 2.0);
  renderCompleteBar(SCREEN_ROW_3_Y, "Rvr", power_rvr, "w", 0.0, 100.0, 2.0);

  display.drawBitmap(0, SCREEN_ROW_4_Y, gamma16_glcd_bmp, 16, 16, 1);
  display.setTextColor(WHITE);
  display.setCursor(44, SCREEN_ROW_4_Y);
  display.println("39   152");
  display.drawBitmap(60, SCREEN_ROW_4_Y, angle8_glcd_bmp, 8, 8, 1);
  display.drawCircle(93, SCREEN_ROW_4_Y, 1, WHITE);

  display.setCursor(44, SCREEN_ROW_5_Y);
  display.println("34 + 9i");
  
  display.display();
}

