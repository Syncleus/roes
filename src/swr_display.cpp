#include "swr_display.h"
#include "swr_constants.h"
#include "swr_strings.h"

#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if (SSD1306_LCDHEIGHT != SCREEN_HEIGHT)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

boolean demo_magnitude_db_increasing = true;
boolean demo_phase_increasing = true;
boolean demo_power_fwd_increasing = true;
boolean demo_power_rvr_increasing = true;

static const PROGMEM unsigned char gamma16_glcd_bmp[] =
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
  B01111110, B00000000
};

static const PROGMEM unsigned char angle8_glcd_bmp[] =
{ B00000000,
  B00000010,
  B00000100,
  B00001000,
  B00010000,
  B00100000,
  B01111110,
  B00000000
};

void displaySetup() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
}

float dbToSwr(float magnitudeDb) {
  float linearFactor = pow(10.0, magnitudeDb / 20.0);

  if ( linearFactor <= 0 )
    return 1.0;

  float pwrs = sqrt(pow(linearFactor, 2.0));
  return (1.0 + pwrs) / (1.0 - pwrs);
}

float polarToComplexA(float magnitudeDb, float phase) {
  return magnitudeDb * cos(phase);
}

float polarToComplexB(float magnitudeDb, float phase) {
  return magnitudeDb * sin(phase);
}

float powerToSwr(float power_fwd, float power_rvr) {
  if ( power_rvr <= 0 )
    return 1.0;
  float pwrs = sqrt(power_rvr / power_fwd);
  return (1.0 + pwrs) / (1.0 - pwrs);
}

float scaleToPercent(float value, float middle, float scale) {
  if ( value == 0 )
    return 0.0;
  else if (isinf(value))
    return 1.0;
  float p = logBased(value / middle, scale);
  float pabs = fabs(p);
  float z = (pow(2.0, pabs) - 1.0) / pow(2.0, pabs);
  if ( p < 0 )
    z = -1.0 * z;
  return (z + 1.0) / 2.0;
}

float scaleToPercent(float value, float value_min, float value_mid, float scale) {
  if (isinf(value))
    return 1.0;
  return scaleToPercent(value - value_min, value_mid - value_min, scale);
}

float logBased(float value, float base) {
  return log10(value) / log10(base);
}

uint8_t percentBar(uint8_t y_offset, float percent) {
  display.fillRect(PERCENT_BAR_TITLE_WIDTH, y_offset, PERCENT_BAR_WIDTH * percent, 15, 1);
  return PERCENT_BAR_TITLE_WIDTH + (PERCENT_BAR_WIDTH * percent);
}

String makeValueLabel(float value) {
  return makeValueLabel(value, NULL);
}

String makeValueLabel(float value, const char* units) {
  String label = String(value);
  if ( isinf(value) )
    return "***";
  uint8_t decimalIndex = label.indexOf(".");
  uint8_t labelLength = label.length();
  if ( decimalIndex > 1 )
    label = label.substring(0, decimalIndex);
  else if(decimalIndex >= 0) {
    label = label.substring(0, (labelLength >= 3 ? 3 : labelLength));
  }
  if( units != NULL )
    label.concat(units);
  return label;
}

uint8_t renderCompleteBar(int8_t y_offset, const char *label, float value, const char *units, float value_min, float value_mid, float scale) {
  display.setCursor(0, y_offset + 4);
  display.println(label);
  float barPercent = scaleToPercent(value, value_min, value_mid, scale);
  uint8_t barEnd = percentBar(y_offset, barPercent);
  String valueLabel = makeValueLabel(value, units);
  uint8_t valueLabelWidth = valueLabel.length() * CHARACTER_WIDTH + 4;
  if ( barEnd < PERCENT_BAR_TITLE_WIDTH + valueLabelWidth + 2) {
    display.setCursor(barEnd + 2, y_offset + 4);
    display.println(valueLabel);
    return barEnd + 2 + valueLabelWidth;
  }
  else {
    display.setTextColor(BLACK);
    display.setCursor(barEnd - valueLabelWidth, y_offset + 4);
    display.println(valueLabel);
    display.setTextColor(WHITE);
    return (barEnd - valueLabelWidth + valueLabelWidth) * -1;
  }
}

void renderPowerSwr(float power_fwd, float power_rvr) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_1_Y, strings(SWR_LABEL), (power_rvr <= power_fwd ? powerToSwr(power_fwd, power_rvr) : powerToSwr(power_fwd, power_fwd)), NULL, 1.0, 2.0, 2.0);
  renderCompleteBar(SCREEN_ROW_2_Y, strings(FWD_LABEL), power_fwd, strings(WATTS_UNIT_LABEL), 0.0, 100.0, 2.0);
  renderCompleteBar(SCREEN_ROW_3_Y, strings(RVR_LABEL), power_rvr, strings(WATTS_UNIT_LABEL), 0.0, 100.0, 2.0);

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

void renderComplexSwr(float magnitudeDb, float phase) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_1_Y, strings(SWR_LABEL), dbToSwr(magnitudeDb), NULL, 1.0, 2.0, 2.0);
  renderCompleteBar(SCREEN_ROW_2_Y, strings(MAG_LABEL), magnitudeDb, strings(DECIBEL_UNIT_LABEL), -30.0, -15.0, 1.5);
  int8_t drawDegreeX = renderCompleteBar(SCREEN_ROW_3_Y, strings(PHS_LABEL), phase, NULL, 0.0, 90.0, 2.0);
  display.drawCircle(abs(drawDegreeX) - 3, SCREEN_ROW_3_Y + 4, 1, (drawDegreeX < 0 ? BLACK : WHITE));

  display.drawBitmap(0, SCREEN_ROW_4_Y, gamma16_glcd_bmp, 16, 16, 1);
  display.setTextColor(WHITE);
  display.setCursor(44, SCREEN_ROW_4_Y);
  float magnitudeLinear = pow(10.0, magnitudeDb / 20.0);
  display.print(makeValueLabel(magnitudeLinear));
  display.print(" ");
  display.println(makeValueLabel(phase));
  display.drawBitmap(60, SCREEN_ROW_4_Y, angle8_glcd_bmp, 8, 8, 1);
  display.drawCircle(93, SCREEN_ROW_4_Y, 1, WHITE);

  display.setCursor(44, SCREEN_ROW_5_Y);
  display.print(makeValueLabel(polarToComplexA(magnitudeDb, phase)));
  display.print(" + ");
  display.print(makeValueLabel(polarToComplexB(magnitudeDb, phase)));
  display.println(" i");

  display.display();
}

void renderStopTransmitting() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(36, SCREEN_ROW_1_Y);
  display.println(strings(STOP_WARNING_LABEL));

  display.setTextSize(1);
  display.setCursor(20, SCREEN_ROW_2_Y);
  display.println(strings(TRANSMITTING_LABEL));

  display.display();
}

void renderCalibration(float power, boolean dummyLoad) {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(4, SCREEN_ROW_1_Y);
  display.println(strings(CALIBRATE_LABEL));

  display.setTextSize(1);
  display.print(strings(CALIBRATE_LINE_1A));
  display.print(makeValueLabel(power, strings(WATTS_UNIT_LABEL)));
  display.println(strings(CALIBRATE_LINE_1B));
  if (dummyLoad)
    display.println(strings(CALIBRATE_LINE_2_DUMMY));
  else
    display.println(strings(CALIBRATE_LINE_2_OPEN));

  display.display();
}

void renderError(String message1, String message2, String message3, String message4) {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(32, SCREEN_ROW_1_Y);
  display.println(strings(ERROR_WARNING_LABEL));

  display.setTextSize(1);
  display.setCursor(0, SCREEN_ROW_2_Y);
  display.println(message1);
  display.println();
  display.println(message2);
  display.println();
  display.println(message3);
  display.println(message4);

  display.display();
}

void updateComplexDemo(float *magnitudeDb, float *phase) {
  if ( demo_magnitude_db_increasing )
    *magnitudeDb += 0.1;
  else
    *magnitudeDb -= 0.1;

  if (*magnitudeDb > 0.0) {
    *magnitudeDb = 0.0;
    demo_magnitude_db_increasing = false;
  }
  else if (*magnitudeDb < -30.0 ) {
    *magnitudeDb = -30.0;
    demo_magnitude_db_increasing = true;
  }

  if ( demo_phase_increasing )
    *phase += 1.0;
  else
    *phase -= 1.0;

  if ( *phase > 180 ) {
    *phase = 180.0;
    demo_phase_increasing = false;
  }
  else if ( *phase < 0 ) {
    *phase = 0.0;
    demo_phase_increasing = true;
  }
}

void updatePowerDemo(float *power_fwd, float *power_rvr) {
  if ( *power_fwd < 10.0 )
    if ( demo_power_fwd_increasing )
      *power_fwd += 0.1;
    else
      *power_fwd -= 0.1;
  else if (*power_fwd >= 100)
    if ( demo_power_fwd_increasing )
      *power_fwd += 10.0;
    else
      *power_fwd -= 10.0;
  else if ( demo_power_fwd_increasing )
    *power_fwd += 1.0;
  else
    *power_fwd -= 1.0;

  if (*power_fwd > 1000.0) {
    *power_fwd = 1000.0;
    demo_power_fwd_increasing = false;
  }
  else if (*power_fwd < 0.0 ) {
    *power_fwd = 0.0;
    demo_power_fwd_increasing = true;
  }

  if ( *power_rvr < 10.0 )
    if ( demo_power_rvr_increasing )
      *power_rvr += 0.2;
    else
      *power_rvr -= 0.2;
  else if (*power_rvr >= 100)
    if ( demo_power_rvr_increasing )
      *power_rvr += 20.0;
    else
      *power_rvr -= 20.0;
  else if ( demo_power_rvr_increasing )
    *power_rvr += 2.0;
  else
    *power_rvr -= 2.0;
  if (*power_rvr > *power_fwd) {
    *power_rvr = *power_fwd;
    demo_power_rvr_increasing = false;
  }
  else if ( *power_rvr < 0.0 ) {
    *power_rvr = 0.0;
    demo_power_rvr_increasing = true;
  }
}
