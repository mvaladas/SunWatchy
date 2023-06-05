#ifndef SUNWATCHY_H
#define SUNWATCHY_H

#include <Watchy.h>
#include "Coordinates.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "Seven_Segment10pt7b.h"
#include "icons.h"
#include <sunset.h>

class SunWatchy : public Watchy {
  using Watchy::Watchy;

private:
  Coordinates coord;
  SunSet sun;

public:
  void drawCircleSegment(int centerX, int centerY, int radius,
                         double startAngle, double endAngle, int patter);
  void drawWatchFace();
  void drawTwilights();
  void drawHands();
  void drawWatchBorder();

  void drawTime();
  void drawDate();
  void drawSteps();
  void drawWeather();
  void drawBattery();
};

#endif
