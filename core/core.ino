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

float power_fwd_test = 0.0;
float power_rvr_test = 0.0;

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
}

void loop() {
  render(power_fwd_test, power_rvr_test);
  delay(25);
  
  if( power_fwd_test < 10.0 )
    power_fwd_test += 0.1;
  else if(power_fwd_test >= 100)
    power_fwd_test += 10;
  else
    power_fwd_test += 1.0;
  if(power_fwd_test > 1000.0)
    power_fwd_test = 0.0;

  if( power_rvr_test < 10.0 )
    power_rvr_test += 0.2;
  else if(power_rvr_test >= 100)
    power_rvr_test += 20;
  else
    power_rvr_test += 2.0;
  if(power_rvr_test > power_fwd_test)
    power_rvr_test = 0.0;
}

#define SCREEN_HEIGHT SSD1306_LCDHEIGHT
#define SCREEN_WIDTH SSD1306_LCDWIDTH
#define PERCENT_BAR_TITLE_WIDTH 20
#define PERCENT_BAR_WIDTH (SCREEN_WIDTH-PERCENT_BAR_TITLE_WIDTH)
void percentBar(int8_t y_offset, float percent) {
  display.fillRect(PERCENT_BAR_TITLE_WIDTH, y_offset, PERCENT_BAR_WIDTH * percent, 15, 1);
}

float logBased(float value, float base) {
  return log10(value)/log10(base);
}

float scaleToPercent(float value, float middle, float scale) {
  float p = logBased(value / middle, scale);
  float pabs = fabs(p);
  float z = (pow(2.0, pabs) - 1.0) / pow(2.0, pabs);
  if( p < 0 )
    z = -1.0 * z;
  return (z + 1.0) / 2.0;
}

float scaleToPercent(float value, float value_min, float value_mid, float scale) {
  return scaleToPercent(value - value_min, value_mid - value_min, scale);
}

String makeValueLabel(float value, String units) {
  String label = String(value);
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
  percentBar(y_offset, scaleToPercent(value, value_min, value_mid, scale));
  display.setTextColor(WHITE);
  display.setCursor(40, y_offset + 4);
  display.println(makeValueLabel(value, units));
  display.setTextColor(WHITE);
}

void render(float power_fwd, float power_rvr) {
  display.clearDisplay();

  renderCompleteBar(0, "SWR", swrFromPower(power_fwd, power_rvr), "", 1.0, 2.0, 2.0);
  renderCompleteBar(16, "Fwd", power_fwd, "w", 0.0, 100.0, 2.0);
  renderCompleteBar(33, "Rvr", power_rvr, "w", 0.0, 100.0, 2.0);

  display.setTextColor(WHITE);
  display.setCursor(0, 49);
  display.println("Refl 3.6 Phase 132");
  display.drawCircle(110, 49, 1, WHITE);

  display.setCursor(32, 57);
  display.println("9i + 34");
  
  display.display();
}

