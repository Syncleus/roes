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

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  render(1.5, 200.0);
}

void loop() {
  
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

String makeLabel(float value, String units) {
  String label = String(value);
  int8_t decimalIndex = label.indexOf(".");
  if( decimalIndex > 1 )
    label = label.substring(0,decimalIndex);
  else
    label = label.substring(0, 3);
  label.concat(units);
  return label;
}

void render(float power_fwd, float power_rvr) {
    // Clear the buffer.
  display.clearDisplay();

  // draw the first ~12 characters in the font
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,4);
  display.println("SWR");
  //display.fillRect(20, 1, 30, 15, 1);
  percentBar(1, 0.5);
  display.setCursor(52,4);
  display.println("2.0");

  display.setCursor(0, 20);
  display.println("Fwd");
  //display.fillRect(20, 16, 90, 15, 1);
  percentBar(16, scaleToPercent(power_fwd, 100.0, 2.0));
  display.setTextColor(WHITE);
  display.setCursor(40, 20);
  display.println(makeLabel(power_fwd, "w"));
  display.setTextColor(WHITE);

  display.setCursor(0, 37);
  display.println("Rvr");
  //display.fillRect(20, 33, 40, 15, 1);
  percentBar(33, scaleToPercent(power_rvr, 100.0, 2.0));
  display.setCursor(62, 37);
  display.println("50w");

  display.setTextColor(WHITE);
  display.setCursor(0, 49);
  display.println("Refl 3.6 Phase 132");
  display.drawCircle(110, 49, 1, WHITE);

  display.setCursor(32, 57);
  display.println("9i + 34");
  
  display.display();
}

