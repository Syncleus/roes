#ifndef _SWR_SMITHCHART_H_
#define _SWR_SMITHCHART_H_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>

void drawSmithChart(Adafruit_ILI9341 display, boolean drawGraticules, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, float magDb, float phase);

#endif /* _SWR_SMITHCHART_H_ */
