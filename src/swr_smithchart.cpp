#include "swr_colors.h"
#include "swr_smithchart.h"
#include "complex.h"
#include <math.h>

struct SmithChartInfo {
  uint16_t x0; // left most x coord containing chart
  uint16_t y0; // top most y coord containing chart
  uint16_t x1; // right most x coord containing chart
  uint16_t y1; // bottom most y coord containing chart
  uint16_t width; //width of containing box
  uint16_t height; //height of containing box
  uint16_t centerX; // x coord of center of chart
  uint16_t centerY; // y coord of center of chart
  uint16_t baseRadius; // radius of the largest containing circle of chart
  uint16_t baseDiameter; // diameter of the largest containing circle of chart
  uint16_t originX; // x coord of open circuit on chart
  uint16_t antioriginX; // x coord of short circuit on chart
  //centerY is the origin and antiorigin y coord
};

struct Axis {
  String label;
  float value;
};

static const Axis xAxis[] =
{
  {"4", 4.0},
  {"", 3.0},
  {"2", 2.0},
  {"", 1.5},
  {"1", 1.0},
  {"", 0.75},
  {"0.5", 0.5},
  {"", 0.25}
};

static const Axis yAxis[] =
{
  {"4", 4.0},
  {"3", 3.0},
  {"2", 2.0},
  {"1.5", 1.5},
  {"1", 1.0},
  {"0.75", 0.75},
  {"0.5", 0.5},
  {"0.25", 0.25}
};

// void drawArc(Adafruit_ILI9341 display, float cx, float cy, float px, float py, float theta, int N, uint16_t color, boolean mirror, float mirrorY)
// {
//     float dx = px - cx;
//     float dy = py - cy;
//     float r2 = dx * dx + dy * dy;
//     float r = sqrt(r2);
//     float ctheta = cos(theta/(N-1));
//     float stheta = sin(theta/(N-1));
//     display.startWrite();
//     display.writePixel(cx + dx, cy + dy, color);
//     for(int i = 1; i != N; ++i)
//     {
//         float dxtemp = ctheta * dx - stheta * dy;
//         dy = stheta * dx + ctheta * dy;
//         dx = dxtemp;
//         float x = cx + dx;
//         float y = cy + dy;
//         display.writePixel(x, y, color);
//         if(mirror)
//           display.writePixel(x, mirrorY + (mirrorY - y), color);
//     }
//     display.endWrite();
// }
//
// void drawArc(Adafruit_ILI9341 display, float cx, float cy, float px, float py, float theta, int N, uint16_t color) {
//   drawArc(display, cx, cy, px, py, theta, N, color, false, 0.0);
// }

void drawArc(Adafruit_ILI9341 display, float cx, float cy, float px, float py, float topY, int N, uint16_t color, boolean mirror, float mirrorY)
{
  float theta = 7.0;
  float dx = px - cx;
  float dy = py - cy;
  float r2 = dx * dx + dy * dy;
  float r = sqrt(r2);
  float ctheta = cos(theta/(N-1));
  float stheta = sin(theta/(N-1));
  display.startWrite();
  display.writePixel(cx + dx, cy + dy, color);
  for(int i = 1; i != N; ++i)
  {
    float dxtemp = ctheta * dx - stheta * dy;
    dy = stheta * dx + ctheta * dy;
    dx = dxtemp;
    float x = cx + dx;
    float y = cy + dy;
    if(y < topY)
      break;
    display.writePixel(x, y, color);
    if(mirror)
      display.writePixel(x, mirrorY + (mirrorY - y), color);
  }
  display.endWrite();
}

float pointLength(float x0, float y0, float x1, float y1) {
  float xDist = abs(x0 - x1);
  float yDist = abs(y0 - y1);
  return sqrt(xDist+xDist + yDist*yDist);
}

void drawSmithChart(Adafruit_ILI9341 display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, float reflMagDb, float reflPhase, float loatMagDb, float loadPhase) {
  SmithChartInfo info;
  info.x0 = x0;
  info.y0 = y0;
  info.x1 = x1;
  info.y1 = y1;
  info.width = (x1 - x0);
  info.centerX = x0 + info.width/2;
  info.height = (y1 - y0);
  info.centerY = y0 + info.height/2;
  if(info.width < info.height)
    info.baseRadius = info.width/2;
  else
    info.baseRadius = info.height/2;
  info.baseDiameter = info.baseRadius * 2;
  info.originX = info.centerX + info.baseRadius;
  info.antioriginX = info.centerX - info.baseRadius;

  display.drawCircle(info.centerX, info.centerY, info.baseRadius, WHITE);
  //drawArc(x + rad, y - rad, rad, 270, 0, WHITE);
  display.setCursor(info.centerX - info.baseRadius + 4, info.centerY + 4);
  display.setTextSize(1);
  display.print("0");
  display.drawLine(info.antioriginX, info.centerY, info.originX, info.centerY, WHITE);

  boolean textUp = false;
  for(int i = 0; i < sizeof(xAxis) / sizeof(Axis); i++) {
    float real = xAxis[i].value;
    String label = xAxis[i].label;
    uint16_t px = info.antioriginX + (-1.0/(real + 1.0) + 1.0) * info.baseDiameter;
    uint16_t cx = px + (info.originX - px) / 2;
    drawArc(display, cx, info.centerY, px, info.centerY, 0.0, 200, WHITE, false, 0.0);

    if( !textUp )
      display.setCursor(px + 4, info.centerY + 4);
    else
      display.setCursor(px + 4, info.centerY - 8);
    display.setTextSize(1);
    display.print(label);

    textUp = !textUp;
  }

  for(int i = 0; i < sizeof(yAxis) / sizeof(Axis); i++) {
    float imag = yAxis[i].value;
    String label = yAxis[i].label;
    float cy = float(info.centerY) - (float(info.baseRadius)/imag);

    //calculate angle
    Complex c1(-1.0, imag);
    Complex c2(1.0, imag);
    Complex intercept = c1/c2;
    float ix = float(info.centerX) + intercept.real() * (float(info.baseRadius));
    float iy = float(info.centerY) - intercept.imag() * (float(info.baseRadius));

    drawArc(display, float(info.originX), cy, float(info.originX), float(info.centerY), iy, 200, WHITE, true, info.centerY);
    display.setCursor(ix, iy);
    display.setTextSize(1);
    display.print(label);
    //mirror onto other side
    display.setCursor(ix, info.centerY + (info.centerY - iy));
    display.print(label);

    //
    // float a = pointLength(float(info.originX), cy, float(info.originX), float(info.centerY));
    // float b = pointLength(float(info.originX), cy, ix, iy);
    // float c = pointLength(ix, iy, float(info.originX), float(info.centerY));
    // float theta = acos((a*a + b*b - c*c) / (2.0*a*b));
    //
    // drawArc(display, float(info.originX), cy, float(info.originX), float(info.centerY), theta, 200, WHITE, true, float(info.centerY));
  }
}
