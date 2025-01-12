#ifndef SUNWATCHY_H
#define SUNWATCHY_H

#include <Watchy.h>
#include "Coordinates.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "Roboto_Medium_10.h"
#include "Seven_Segment10pt7b.h"
#include "icons.h"
#include <sunset.h>

class SunWatchy : public Watchy {
  using Watchy::Watchy;

private:
  Coordinates coord;
  SunSet sun;

public:
  void drawCircleSegment(const int centerX, const int centerY, const int radius,
                         const double startAngle, const double endAngle, const int patter);
  void drawWatchFace();
  void drawTwilights();
  void drawHourHand();
  void drawMinuteHand();
  void drawWatchBorder();

  void drawTime();
  void drawDate();
  void drawSteps();
  void drawWeather();
  void drawTopRight();


};

#endif
