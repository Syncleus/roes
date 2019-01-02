#include "swr_display.h"
#include "swr_constants.h"
#include "swr_strings.h"
#include "swr_power.h"
#include "swr_smithchart.h"
#include "swr_colors.h"
#include <math.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>

//We intentionally use the screen in landscape mode so height and width are
//switched
#if (ILI9341_TFTWIDTH != SCREEN_WIDTH)
#error("Height incorrect, please fix Adafruit_ILI9341.h!");
#endif
#if (ILI9341_TFTHEIGHT != SCREEN_HEIGHT)
#error("Height incorrect, please fix Adafruit_ILI9341.h!");
#endif

//configure display
// The display uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();
// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

boolean demo_magnitude_db_increasing = true;
boolean demo_phase_increasing = true;
boolean demo_power_fwd_increasing = true;
boolean demo_power_rvr_increasing = true;
boolean clear_smith_chart = true;

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

static const PROGMEM unsigned char gamma16_inv_glcd_bmp[] =
{ B10000000, B00000001,
  B11000000, B00000001,
  B11100111, B11111001,
  B11100111, B11111101,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11100111, B11111111,
  B11000011, B11111111,
  B10000001, B11111111
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

static const PROGMEM unsigned char angle8_inv_glcd_bmp[] =
{ B11111111,
  B11111101,
  B11111011,
  B11110111,
  B11101111,
  B11011111,
  B10000001,
  B11111111
};

static const PROGMEM unsigned char degree_glcd_bmp[] =
{ B01000000,
  B10100000,
  B01000000,
  B00000000,
  B00000000,
  B00100000,
  B00000000,
  B00000000
};

static const PROGMEM unsigned char degree_inv_glcd_bmp[] =
{ B10111111,
  B01011111,
  B10111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111,
  B11111111
};

void displaySetup() {
  display.begin();

  if (! ctp.begin(40)) {
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  display.fillScreen(BLACK);
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

uint8_t percentBar(uint8_t y_offset, float percent, uint16_t color) {
  display.fillRect(PERCENT_BAR_TITLE_WIDTH, y_offset, PERCENT_BAR_WIDTH * percent, 15, color);
  return PERCENT_BAR_TITLE_WIDTH + (PERCENT_BAR_WIDTH * percent);
}

uint8_t percentBar(uint8_t y_offset, float percent) {
  return percentBar(y_offset, percent, WHITE);
}

String makeValueLabel(float value) {
  return makeValueLabel(value, NULL);
}

String makeValueLabel(float value, const char* units) {
  String label = String(value);
  if ( isinf(value) || isnan(value) )
    return String("***");
  int16_t decimalIndex = label.indexOf(".");

  String out;
  if(decimalIndex >= 4)
    out = label.substring(0, decimalIndex);
  else if(decimalIndex >= 0)
    out = label.substring(0, decimalIndex + 1 + (4 - decimalIndex)) + label.substring(decimalIndex, decimalIndex);
  else
    out = label;

  if( units != NULL )
    out.concat(units);
  return out;
}

uint8_t renderCompleteBar(int8_t y_offset, const char* label, float value, const char* units, float value_min, float value_mid, float scale, boolean inverse, uint16_t fg, uint16_t bg) {
  display.setTextColor(WHITE, bg);
  display.setCursor(0, y_offset + 4);
  if(isnan(value)) {
    display.print(String(label)+String(" ***"));
    return PERCENT_BAR_TITLE_WIDTH + (4 * CHARACTER_WIDTH) + 4;
  }
  display.print(label);
  float barValue = (inverse ? -1.0 * value : value);
  float barPercent = scaleToPercent(barValue, value_min, value_mid, scale);
  uint8_t barEnd = percentBar(y_offset, barPercent, fg);
  display.fillRect(barEnd, y_offset, SCREEN_HEIGHT-barEnd, 15, bg);
  String valueLabel = makeValueLabel(value, units);
  uint8_t valueLabelWidth = valueLabel.length() * CHARACTER_WIDTH + 4;
  if ( barEnd >= PERCENT_BAR_TITLE_WIDTH + valueLabelWidth + 2) {
    display.setTextColor(bg, fg);
    display.setCursor(barEnd - valueLabelWidth, y_offset + 4);
    display.println(valueLabel);
    display.setTextColor(fg, bg);
    return (barEnd - valueLabelWidth + valueLabelWidth) * -1;
  }
  else {
    display.setTextColor(fg, bg);
    display.setCursor(barEnd + 2, y_offset + 4);
    display.println(valueLabel);
    return barEnd + 2 + valueLabelWidth;
  }
}

uint8_t renderCompleteBar(int8_t y_offset, const char *label, float value, const char *units, float value_min, float value_mid, float scale) {
  return renderCompleteBar(y_offset, label, value, units, value_min, value_mid, scale, false, WHITE, BLACK);
}

uint8_t renderCompleteBar(int8_t y_offset, const char *label, float value, const char *units, float value_min, float value_mid, float scale, boolean inverse) {
  return renderCompleteBar(y_offset, label, value, units, value_min, value_mid, scale, inverse, WHITE, BLACK);
}

void prepareRender() {
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
}

void renderSwr(float swr) {
  uint16_t fg;
  if( swr < 1.5 )
    fg = BLUE;
  else if( swr >= 1.5 && swr < 2.0 )
    fg = DARKGREEN;
  else if( swr >= 2.0 && swr < 3.0 )
    fg = ORANGE;
  else
    fg = RED;

  renderCompleteBar(SCREEN_ROW_1_Y, SWR_LABEL, swr, NULL, 1.0, 2.0, 2.0, false, fg, BLACK);
}

void finishRender() {
  //nothing to do here for this type of display
}

void renderPowerBars(float power_fwd, float power_rvr) {
  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_2_Y, FWD_LABEL, power_fwd, WATTS_UNIT_LABEL, 0.0, 100.0, 2.0);
  renderCompleteBar(SCREEN_ROW_3_Y, RVR_LABEL, power_rvr, WATTS_UNIT_LABEL, 0.0, 100.0, 2.0);
}

void renderReflectionBars(float magnitudeDb, float phase) {
  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_2_Y, RL_LABEL, -1.0 * magnitudeDb, DECIBEL_UNIT_LABEL, -30.0, -15.0, 1.5, true);
  int8_t drawDegreeX = renderCompleteBar(SCREEN_ROW_3_Y, PHS_LABEL, phase, NULL, -180.0, 0.0, 2.0);
  display.drawCircle(abs(drawDegreeX) - 3, SCREEN_ROW_3_Y + 4, 1, (drawDegreeX < 0 ? BLACK : WHITE));
}

void renderPowerText(float power_fwd, float power_rvr) {
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, SCREEN_ROW_4_Y + 4);
  display.println(POWER_LABEL);

  String blank = String("     ");

  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_4_Y);
  String forwardTitle = POWER_FWD_LABEL;
  display.print(forwardTitle);
  String forwardText = String(ONE_SPACE) + makeValueLabel(power_fwd, WATTS_UNIT_LABEL) + String(ONE_SPACE) + makeValueLabel(powerToDbm(power_fwd), DBM_UNIT_LABEL) + blank;
  display.println(forwardText.c_str());

  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_4H_Y);
  String reverseTitle = POWER_RVR_LABEL;
  display.print(reverseTitle);
  String reverseText = String(ONE_SPACE) + makeValueLabel(power_rvr, WATTS_UNIT_LABEL) + String(ONE_SPACE) + makeValueLabel(powerToDbm(power_rvr), DBM_UNIT_LABEL) + blank;
  display.println(reverseText);
}

void clearSmithChart() {
  clear_smith_chart = true;
}

void renderSmithChart(float magDb, float phase) {
  drawSmithChart(display, clear_smith_chart, 0, SCREEN_ROW_GRFX_Y, SCREEN_WIDTH, SCREEN_ROW_GRFX_Y_END, magDb, phase);
  if(clear_smith_chart)
    clear_smith_chart = false;
}

void renderLoadText(float magnitudeDb, float phase) {
  //calculate magnitude as a linear value (no longer in decibels)
  float magnitudeLinear = pow(10.0, magnitudeDb / 20.0);

  ///////////////////////////////////
  // Header for Reflection Coefficient (Gamma)
  ///////////////////////////////////
  display.drawBitmap(0, SCREEN_ROW_5_Y, gamma16_glcd_bmp, 16, 16, CYAN);
  display.drawBitmap(0, SCREEN_ROW_5_Y, gamma16_inv_glcd_bmp, 16, 16, BLACK);

  ////////////////////////////////
  // Print polar coordinates
  ////////////////////////////////
  display.setTextColor(CYAN, BLACK);
  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_5_Y);
  display.setTextSize(1);

  String magnitudeReflText = makeValueLabel(magnitudeLinear) + String(ONE_SPACE);
  display.print(magnitudeReflText);

  uint16_t angleReflPos = PERCENT_BAR_TITLE_WIDTH + CHARACTER_WIDTH * magnitudeReflText.length();
  display.drawBitmap(angleReflPos, SCREEN_ROW_5_Y, angle8_glcd_bmp, 8, 8, CYAN);
  display.drawBitmap(angleReflPos, SCREEN_ROW_5_Y, angle8_inv_glcd_bmp, 8, 8, BLACK);

  uint16_t phaseReflPos = angleReflPos + 8;
  display.setCursor(phaseReflPos, SCREEN_ROW_5_Y);
  String phaseReflText = String(ONE_SPACE) + makeValueLabel(phase);
  display.print(phaseReflText);

  uint16_t degreeReflPos = phaseReflPos + phaseReflText.length() * CHARACTER_WIDTH;
  display.drawBitmap(degreeReflPos, SCREEN_ROW_5_Y, degree_glcd_bmp, 8, 8, CYAN);
  display.drawBitmap(degreeReflPos, SCREEN_ROW_5_Y, degree_inv_glcd_bmp, 8, 8, BLACK);

  uint16_t polarReflMarginPos = degreeReflPos + 8;
  display.setCursor(polarReflMarginPos, SCREEN_ROW_5_Y);
  display.print(THREE_SPACE);

  ////////////////////////////////////
  // Print Complex Number Form
  ///////////////////////////////////
  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_5H_Y);

  Complex reflComplex = polarToComplex(magnitudeLinear, phase);
  String complexReflText = makeValueLabel(reflComplex.real()) + String(COMPLEX_PLUS_STRING) + makeValueLabel(reflComplex.imag()) + String(COMPLEX_I_STRING) + String(THREE_SPACE);
  display.print(complexReflText);

  ///////////////////////////////////
  // Header for Complex Load (Z)
  ///////////////////////////////////
  display.setTextColor(MAGENTA, BLACK);
  display.setTextSize(2);
  display.setCursor(SCREEN_WIDTH/2, SCREEN_ROW_5_Y);
  display.print("Z");

  ////////////////////////////////
  // Print polar coordinates
  ////////////////////////////////
  uint16_t zTitlePos = SCREEN_WIDTH/2 + PERCENT_BAR_TITLE_WIDTH;
  display.setCursor(zTitlePos, SCREEN_ROW_5_Y);
  display.setTextSize(1);
  Complex loadZ = complexLoadFromReflection(magnitudeLinear, phase);

  String magnitudeText = makeValueLabel(loadZ.modulus()) + String(ONE_SPACE);
  display.print(magnitudeText);

  uint16_t anglePos = zTitlePos + CHARACTER_WIDTH * magnitudeText.length();
  display.drawBitmap(anglePos, SCREEN_ROW_5_Y, angle8_glcd_bmp, 8, 8, MAGENTA);
  display.drawBitmap(anglePos, SCREEN_ROW_5_Y, angle8_inv_glcd_bmp, 8, 8, BLACK);

  uint16_t phasePos = anglePos + 8;
  display.setCursor(phasePos, SCREEN_ROW_5_Y);
  String phaseText = String(ONE_SPACE) + makeValueLabel(loadZ.phase());
  display.print(phaseText);

  uint16_t degreePos = phasePos + phaseText.length() * CHARACTER_WIDTH;
  display.drawBitmap(degreePos, SCREEN_ROW_5_Y, degree_glcd_bmp, 8, 8, MAGENTA);
  display.drawBitmap(degreePos, SCREEN_ROW_5_Y, degree_inv_glcd_bmp, 8, 8, BLACK);

  uint16_t polarMarginPos = degreePos + 8;
  display.setCursor(polarMarginPos, SCREEN_ROW_5_Y);
  display.print(THREE_SPACE);

  ////////////////////////////////////
  // Print Complex Number Form
  ///////////////////////////////////
  display.setCursor(zTitlePos, SCREEN_ROW_5H_Y);

  String complexText = makeValueLabel(loadZ.real()) + String(COMPLEX_PLUS_STRING) + makeValueLabel(loadZ.imag()) + String(COMPLEX_I_STRING) + String(THREE_SPACE);
  display.print(complexText);
}

void renderStopTransmitting() {
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(36, SCREEN_ROW_1_Y);
  display.println(STOP_WARNING_LABEL);

  display.setTextSize(1);
  display.setCursor(20, SCREEN_ROW_2_Y);
  display.println(TRANSMITTING_LABEL);
}

void renderCalibration(float power, boolean dummyLoad) {
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(4, SCREEN_ROW_1_Y);
  display.println(CALIBRATE_LABEL);

  display.setTextSize(1);
  display.print(CALIBRATE_LINE_1A);
  display.print(makeValueLabel(power, WATTS_UNIT_LABEL));
  display.println(CALIBRATE_LINE_1B);
  if (dummyLoad)
    display.println(CALIBRATE_LINE_2_DUMMY);
  else
    display.println(CALIBRATE_LINE_2_OPEN);
}

void renderError(String message1, String message2, String message3, String message4) {
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(32, SCREEN_ROW_1_Y);
  display.println(ERROR_WARNING_LABEL);

  display.setTextSize(1);
  display.setCursor(0, SCREEN_ROW_2_Y);
  display.println(message1);
  display.println();
  display.println(message2);
  display.println();
  display.println(message3);
  display.println(message4);
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

  if ( *phase > 180.0 ) {
    *phase = 180.0;
    demo_phase_increasing = false;
  }
  else if ( *phase < -180.0 ) {
    *phase = -180.0;
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
