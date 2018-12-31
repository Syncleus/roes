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

  renderCompleteBar(SCREEN_ROW_1_Y, strings(SWR_LABEL), swr, NULL, 1.0, 2.0, 2.0, false, fg, BLACK);
}

void finishRender() {
  //nothing to do here for this type of display
}

void renderPowerBars(float power_fwd, float power_rvr) {
  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_2_Y, strings(FWD_LABEL), power_fwd, strings(WATTS_UNIT_LABEL), 0.0, 100.0, 2.0);
  renderCompleteBar(SCREEN_ROW_3_Y, strings(RVR_LABEL), power_rvr, strings(WATTS_UNIT_LABEL), 0.0, 100.0, 2.0);
}

void renderReflectionBars(float magnitudeDb, float phase) {
  //make sure power_rvr isnt higher than power_fwd
  renderCompleteBar(SCREEN_ROW_2_Y, strings(RL_LABEL), -1.0 * magnitudeDb, strings(DECIBEL_UNIT_LABEL), -30.0, -15.0, 1.5, true);
  int8_t drawDegreeX = renderCompleteBar(SCREEN_ROW_3_Y, strings(PHS_LABEL), phase, NULL, -180.0, 0.0, 2.0);
  display.drawCircle(abs(drawDegreeX) - 3, SCREEN_ROW_3_Y + 4, 1, (drawDegreeX < 0 ? BLACK : WHITE));
}

void renderPowerText(float power_fwd, float power_rvr) {
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, SCREEN_ROW_4_Y + 4);
  display.println(strings(POWER_LABEL));

  String blank = String("     ");

  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_4_Y);
  String forwardTitle = strings(POWER_FWD_LABEL);
  display.print(forwardTitle);
  String forwardText = String(strings(SINGLE_SPACE)) + makeValueLabel(power_fwd, strings(WATTS_UNIT_LABEL)) + String(strings(SINGLE_SPACE)) + makeValueLabel(powerToDbm(power_fwd), strings(DBM_UNIT_LABEL)) + blank;
  display.println(forwardText.c_str());

  display.setCursor(PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_4H_Y);
  String reverseTitle = strings(POWER_RVR_LABEL);
  display.print(reverseTitle);
  String reverseText = String(strings(SINGLE_SPACE)) + makeValueLabel(power_rvr, strings(WATTS_UNIT_LABEL)) + String(strings(SINGLE_SPACE)) + makeValueLabel(powerToDbm(power_rvr), strings(DBM_UNIT_LABEL)) + blank;
  display.println(reverseText);
}

void renderSmithChart(float reflMagDb, float reflPhase, float loatMagDb, float loadPhase) {
  drawSmithChart(display, 0, SCREEN_ROW_GRFX_Y, SCREEN_WIDTH, SCREEN_ROW_GRFX_Y_END, reflMagDb, reflPhase, loatMagDb, loadPhase);
}

void renderReflectionText(float magnitudeDb, float phase) {
  display.drawBitmap(0, SCREEN_ROW_5_Y, gamma16_glcd_bmp, 16, 16, WHITE);

  float magnitudeLinear = pow(10.0, magnitudeDb / 20.0);
  String cartesianText = String("   ") + makeValueLabel(magnitudeLinear) + String("   ") + makeValueLabel(phase) + String("   ");
  uint8_t cartesianTextWidth = cartesianText.length() * CHARACTER_WIDTH;
  uint8_t cartesianLeftMargin = ((SCREEN_WIDTH - cartesianTextWidth - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH;
  display.setTextColor(WHITE, BLACK);
  display.setCursor(cartesianLeftMargin, SCREEN_ROW_5_Y);
  display.println(cartesianText);
  display.drawBitmap(((SCREEN_WIDTH - 8 - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_5_Y, angle8_glcd_bmp, 8, 8, WHITE);
  display.drawCircle(cartesianTextWidth + cartesianLeftMargin - 17, SCREEN_ROW_5_Y, 1, WHITE);

  Complex reflComplex = polarToComplex(magnitudeLinear, phase);
  String complexText = String("   ") + makeValueLabel(reflComplex.real()) + String(" + ") + makeValueLabel(reflComplex.imag()) + String("i") + String("   ");
  uint8_t complexTextWidth = complexText.length() * CHARACTER_WIDTH;
  uint8_t complexLeftMargin = ((SCREEN_WIDTH - complexTextWidth - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH;
  display.setCursor(complexLeftMargin, SCREEN_ROW_5H_Y);
  display.println(complexText);
}

void renderLoadZText(float magnitudeDb, float phase) {
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);
  display.setCursor(0, SCREEN_ROW_6_Y);
  display.print("Z");

  display.setTextSize(1);
  float magnitudeLinear = pow(10.0, magnitudeDb / 20.0);
  Complex loadZ = complexLoadFromReflection(magnitudeLinear, phase);

  String cartesianText = makeValueLabel(loadZ.modulus()) + String("   ") + makeValueLabel(loadZ.phase());
  uint8_t cartesianTextWidth = cartesianText.length() * CHARACTER_WIDTH;
  uint8_t cartesianLeftMargin = ((SCREEN_WIDTH - cartesianTextWidth - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH;
  display.setTextColor(WHITE, BLACK);
  display.setCursor(cartesianLeftMargin, SCREEN_ROW_6_Y);
  display.println(cartesianText);
  display.drawBitmap(((SCREEN_WIDTH - 8 - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH, SCREEN_ROW_6_Y, angle8_glcd_bmp, 8, 8, WHITE);
  display.drawCircle(cartesianTextWidth + cartesianLeftMargin + 1, SCREEN_ROW_6_Y, 1, WHITE);

  String complexText = makeValueLabel(loadZ.real()) + String(" + ") + makeValueLabel(loadZ.imag()) + String(" i");
  uint8_t complexTextWidth = complexText.length() * CHARACTER_WIDTH;
  uint8_t complexLeftMargin = ((SCREEN_WIDTH - complexTextWidth - PERCENT_BAR_TITLE_WIDTH) / 2) + PERCENT_BAR_TITLE_WIDTH;
  display.setCursor(complexLeftMargin, SCREEN_ROW_6H_Y);
  display.println(complexText);
}

void renderStopTransmitting() {
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(36, SCREEN_ROW_1_Y);
  display.println(strings(STOP_WARNING_LABEL));

  display.setTextSize(1);
  display.setCursor(20, SCREEN_ROW_2_Y);
  display.println(strings(TRANSMITTING_LABEL));
}

void renderCalibration(float power, boolean dummyLoad) {
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
}

void renderError(String message1, String message2, String message3, String message4) {
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
