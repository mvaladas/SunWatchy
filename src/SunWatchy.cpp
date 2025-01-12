#include "SunWatchy.h"
#include "GxEPD2.h"
#include "freertos/portable.h"
#include "stdint.h"
#include <Watchy.h>
#include <Watchy.cpp>
#include <sunset.h>

// Fonts
#include "Born2bSportyV2.h"
#include "Digital7.h"
#include "fz5x5.h"
#include "tiny4x5.h"

// Icons
#include "BattNumbers.h"

#define DARKMODE false
#define DEG2RAD 0.0174532925
#define LAT 49.85959500649175 // Darmstadt
#define LON 8.639179730728287 // Darmstadt
#define CLOCK_RADIUS 75
#define MINIRADIUS 3
#define MIN2RAD (DEG2RAD * 360.0 / (24.0 * 60.0))

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

const float BATTERY_MAX_V = 4.20; // Originally 4.1
const float BATTERY_MIN_V = 3.6;  // Originally 3.8

int grayPatterns[4][4][4] = {{
                                 {0, 0, 0, 0},
                                 {1, 0, 1, 0},
                                 {0, 0, 0, 0},
                                 {1, 0, 1, 0},
                             },
                             {
                                 {0, 1, 0, 1},
                                 {1, 0, 1, 0},
                                 {0, 1, 0, 1},
                                 {1, 0, 1, 0},
                             },
                             {
                                 {0, 1, 0, 1},
                                 {1, 1, 1, 1},
                                 {0, 1, 0, 1},
                                 {1, 1, 1, 1},
                             },
                             {
                                 {0, 1, 1, 1},
                                 {1, 1, 1, 1},
                                 {1, 1, 0, 1},
                                 {1, 1, 1, 1},
                             }};

// Fill in a semi-circle element
void SunWatchy::drawCircleSegment(const int centerX, const int centerY, const int radius,
                                  const double startAngle, const double endAngle,
                                  const int pattern) {
  int x, y;
  double twoPi = 2 * M_PI;

  // Normalize angles to be between 0 and 2 * pi
  int startA = fmod(startAngle, twoPi);
  int endA = fmod(endAngle, twoPi);

  if (startA < 0)
    startA += twoPi;
  if (endA < 0)
    endA += twoPi;

  // Iterate through each pixel in the circle segment
  for (y = centerY - radius; y <= centerY + radius; y++) {
    for (x = centerX - radius; x <= centerX + radius; x++) {
      int dx = x - centerX;
      int dy = y - centerY;

      // Check if the pixel is within the circle segment
      int distanceSquared = dx * dx + dy * dy;
      double angle = atan2(dy, dx);

      if (distanceSquared <= radius * radius) {
        // Normalize angle to be between 0 and 2 * pi
        if (angle < 0)
          angle += twoPi;

        // Handle the case where startAngle > endAngle
        if (startAngle > endAngle) {
          if ((angle >= startAngle || angle <= endAngle) &&
              !(angle > endAngle && angle < startAngle)) {
            // Pixel is inside the circle segment, do something with it (e.g.,
            // draw it)
            int numRows = sizeof(grayPatterns[pattern]) / sizeof(grayPatterns[pattern][0]);
            int numCols = sizeof(grayPatterns[pattern][0]) / sizeof(grayPatterns[pattern][0][0]);
            uint16_t color =
                grayPatterns[pattern][x % numRows][y % numCols] == 0 ? GxEPD_WHITE : GxEPD_BLACK;
            display.drawPixel(x, y, color);
          }
        } else {
          if (angle >= startAngle && angle <= endAngle) {
            // Pixel is inside the circle segment, do something with it (e.g.,
            // draw it)
            int numRows = sizeof(grayPatterns[pattern]) / sizeof(grayPatterns[pattern][0]);
            int numCols = sizeof(grayPatterns[pattern][0]) / sizeof(grayPatterns[pattern][0][0]);
            uint16_t color =
                grayPatterns[pattern][x % numRows][y % numCols] == 0 ? GxEPD_WHITE : GxEPD_BLACK;
            display.drawPixel(x, y, color);
          }
        }
      }
    }
  }
}

void SunWatchy::drawTwilights() {
  sun.setPosition(LAT, LON, (int)(gmtOffset / 3600));

  double civilsunrise;
  double civilsunset;
  double nauticalsunrise;
  double nauticalsunset;
  double astrosunrise;
  double astrosunset;
  double sunrise;
  double sunset;

  sun.setCurrentDate(currentTime.Year, currentTime.Month, currentTime.Day);
  /* sun.setCurrentDate(2022, 12, 21); */

  sunrise = sun.calcSunrise();
  sunset = sun.calcSunset();
  civilsunrise = sun.calcCivilSunrise();
  civilsunset = sun.calcCivilSunset();
  nauticalsunrise = sun.calcNauticalSunrise();
  nauticalsunset = sun.calcNauticalSunset();
  astrosunrise = sun.calcAstronomicalSunrise();
  astrosunset = sun.calcAstronomicalSunset();

  float riseAng = sunrise * MIN2RAD - PI / 2;
  float setAng = sunset * MIN2RAD - PI / 2;
  float civilRiseAng = civilsunrise * MIN2RAD - PI / 2;
  float civilSetAng = civilsunset * MIN2RAD - PI / 2;
  float nauticalRiseAng = nauticalsunrise * MIN2RAD - PI / 2;
  float nauticalSetAng = nauticalsunset * MIN2RAD - PI / 2;
  float astroRiseAng = astrosunrise * MIN2RAD - PI / 2;
  float astroSetAng = astrosunset * MIN2RAD - PI / 2;

  drawCircleSegment(100, 100, CLOCK_RADIUS - MINIRADIUS, setAng, riseAng, 0);
  drawCircleSegment(100, 100, CLOCK_RADIUS - MINIRADIUS, civilSetAng, civilRiseAng, 1);
  drawCircleSegment(100, 100, CLOCK_RADIUS - MINIRADIUS, nauticalSetAng, nauticalRiseAng, 2);
  if (!isnan(astroRiseAng) || !isnan(astroSetAng)) {
    drawCircleSegment(100, 100, CLOCK_RADIUS - MINIRADIUS, astroSetAng, astroRiseAng, 3);
  }
}
void SunWatchy::drawWatchBorder() {
  /* for (int i = 0; i < 5; i++) { */
  /*   display.drawCircle(100, 100, CLOCK_RADIUS + i, GxEPD_BLACK); */
  /* } */

  /* display.fillRect(96, 20, 8, 16, GxEPD_BLACK); */
  /* display.fillRect(96, 164, 8, 16, GxEPD_BLACK); */
  /* display.fillRect(20, 96, 16, 8, GxEPD_BLACK); */
  /* display.fillRect(164, 96, 16, 8, GxEPD_BLACK); */

  for (int i = 0; i < 24; i++) {
    int x = 100 + cos(radians(i * 360 / 24)) * (CLOCK_RADIUS - MINIRADIUS - 5);
    int y = 100 + sin(radians(i * 360 / 24)) * (CLOCK_RADIUS - MINIRADIUS - 5);
    display.fillCircle(x, y, MINIRADIUS + 1, GxEPD_WHITE);
    display.fillCircle(x, y, MINIRADIUS, GxEPD_BLACK);
  }

  /* for (int i = 0; i < 4; i++) { */
  /*   display.drawCircle(100, 100, CLOCK_RADIUS + i, GxEPD_BLACK); */
  /* } */
}

void SunWatchy::drawHourHand() {
  int handSize = 5;
  Coordinates t1, t2, t3, t4;
  float hour = currentTime.Hour;
  float minutes = (currentTime.Minute + hour * 60.0);
  float angle = (minutes * 2 * PI / (24.0 * 60.0));

  t1.fromPolar(handSize, angle);
  t2.fromPolar(handSize, angle + PI);
  t3.fromPolar(CLOCK_RADIUS - 7, angle - PI / 2);
  t4.fromPolar(handSize * 4, angle + PI / 2);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t3.getX() + 100, t3.getY() + 100, GxEPD_WHITE);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t4.getX() + 100, t4.getY() + 100, GxEPD_WHITE);
  display.fillCircle(100, 100, handSize + 3, GxEPD_WHITE);

  handSize -= 1;
  t1.fromPolar(handSize, angle);
  t2.fromPolar(handSize, angle + PI);
  t3.fromPolar(CLOCK_RADIUS - 10, angle - PI / 2);
  t4.fromPolar(handSize * 4, angle + PI / 2);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t3.getX() + 100, t3.getY() + 100, GxEPD_BLACK);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t4.getX() + 100, t4.getY() + 100, GxEPD_BLACK);
  display.fillCircle(100, 100, handSize + 3, GxEPD_BLACK);
}

void SunWatchy::drawMinuteHand() {
  int handSize = 5;
  Coordinates t1, t2, t3, t4;
  float minutes = (currentTime.Minute);
  float angle = (minutes * 2 * PI / 60.0);

  t1.fromPolar(handSize, angle);
  t2.fromPolar(handSize, angle + PI);
  t3.fromPolar(CLOCK_RADIUS - 10, angle - PI / 2);
  t4.fromPolar(handSize * 4, angle + PI / 2);

  display.fillCircle(100, 100, handSize + 3, GxEPD_BLACK);
  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t3.getX() + 100, t3.getY() + 100, GxEPD_BLACK);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100, t2.getY() + 100,
                       t4.getX() + 100, t4.getY() + 100, GxEPD_BLACK);
}

void SunWatchy::drawWatchFace() {
  display.fillScreen(GxEPD_BLACK);
  display.fillCircle(100, 100, CLOCK_RADIUS, GxEPD_WHITE);
  display.drawCircle(100, 100, CLOCK_RADIUS - 2, GxEPD_BLACK);
  drawTwilights();
  drawHourHand();
  /* drawMinuteHand(); */
  drawWatchBorder();

  /* drawTime(); */
  drawDate();
  drawTopRight();
  /* drawSteps(); */
  /* drawWeather(); */

  /* if (BLE_CONFIGURED) { */
  /*   display.drawBitmap(110, 3, bluetooth, 13, 21, !DARKMODE ? GxEPD_WHITE : GxEPD_BLACK); */
  /* } */
}

void SunWatchy::drawTime() {
  // Text bounds calculation
  int16_t x1, y1;
  uint16_t w, h;

  String time;

  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(1);
  display.setFont(&Digital724pt7b);
  int displayHour;
  if (HOUR_12_24 == 12) {
    displayHour = ((currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = currentTime.Hour;
  }
  if (displayHour < 10) {
    /* display.print("0"); */
    time += "0";
  }
  /* display.print(displayHour); */
  /* display.print(":"); */
  time += displayHour;
  time += ":";
  if (currentTime.Minute < 10) {
    /* display.print("0"); */
    time += "0";
  }
  /* display.println(currentTime.Minute); */
  time += currentTime.Minute;

  display.getTextBounds(time, 0, 100, &x1, &y1, &w, &h);
  display.setCursor(100 - x1 - w / 2, 70);
  display.setTextColor(GxEPD_BLACK);
  display.print(time);
}

void SunWatchy::drawDate() {
  display.setTextSize(1);
  display.setFont(&Born2bSportyV210pt7b);
  display.setTextColor(GxEPD_WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  /* String dayOfWeek = dayStr(currentTime.Wday); */
  /* display.getTextBounds(dayOfWeek, 5, 85, &x1, &y1, &w, &h); */
  /* if (currentTime.Wday == 4) { */
  /*   w = w - 5; */
  /* } */
  /* display.setCursor(85 - w, 85); */
  /* display.println(dayOfWeek); */
  /**/
  String month = monthShortStr(currentTime.Month);
  /* display.getTextBounds(month, 60, 110, &x1, &y1, &w, &h); */
  /* display.setCursor(85 - w, 110); */
  /* display.println(month); */

  /* display.setFont(&DSEG7_Classic_Bold_25); */
  /* display.setCursor(5, 120); */
  /* if (currentTime.Day < 10) { */
  /*   display.print("0"); */
  /* } */

  String daymon;
  daymon += currentTime.Day;
  daymon += ". " + month + ".";
  display.getTextBounds(daymon, 2, 2, &x1, &y1, &w, &h);
  display.setCursor(2, 2 + h);
  display.println(daymon);

  int dayH = h;

  String weekday;
  weekday += dayShortStr(currentTime.Wday);
  weekday += ".";
  display.getTextBounds(daymon, 2, 2 + dayH + 2, &x1, &y1, &w, &h);
  display.setCursor(2, 4 + dayH + h);
  display.println(weekday);
}
void SunWatchy::drawSteps() {
  // reset step counter at midnight
  if (currentTime.Hour == 0 && currentTime.Minute == 0) {
    sensor.resetStepCounter();
  }
  uint32_t stepCount = sensor.getCounter();
  display.drawBitmap(10, 165, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  display.setCursor(35, 190);
  display.println(stepCount);
}
void SunWatchy::drawTopRight() {
  int8_t batteryLevel = 0;
  float VBAT = getBatteryVoltage();

  int16_t x1, y1;
  uint16_t w, h;

  display.setTextColor(GxEPD_WHITE);
  display.setTextSize(1);
  display.setFont(&fz5x515pt7b);
  int battPercent = (int)(100 * (VBAT - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V));
  String percent = String(battPercent) + "%";
  display.getTextBounds(percent, 0, 0, &x1, &y1, &w, &h);

  int16_t textX = 193 - w - x1 - (h / 2);
  int16_t textY = 2 + h + (h / 2);
  display.setCursor(textX, textY);
  display.print(percent);

  display.drawRect(textX - 3, textY - h - 3, w + 7, h + 7, GxEPD_WHITE);
  display.fillRect(193, textY - h - 3 + ((h + 7) / 3), 3, (h + 7) / 3, GxEPD_WHITE);

  // Draw Wifi Symbol
  display.drawBitmap(textX - 5 - 28, 2, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, GxEPD_WHITE);

  weatherData currentWeather = getWeatherData();

  int8_t temperature = currentWeather.temperature;
  String degrees = String(temperature) + " C";
  display.setFont(&Born2bSportyV210pt7b);
  display.setTextSize(1);
  display.getTextBounds(degrees, 2, 2, &x1, &y1, &w, &h);
  display.setCursor(180 - w, textY + 10 + h);
  display.println(degrees);
}

void SunWatchy::drawWeather() {

  weatherData currentWeather = getWeatherData();

  int8_t temperature = currentWeather.temperature;
  int16_t weatherConditionCode = currentWeather.weatherConditionCode;

  display.setFont(&DSEG7_Classic_Regular_39);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);
  if (159 - w - x1 > 87) {
    display.setCursor(159 - w - x1, 150);
  } else {
    display.setFont(&DSEG7_Classic_Bold_25);
    display.getTextBounds(String(temperature), 0, 0, &x1, &y1, &w, &h);
    display.setCursor(159 - w - x1, 136);
  }
  display.println(temperature);
  display.drawBitmap(165, 110, currentWeather.isMetric ? celsius : fahrenheit, 26, 20,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  const unsigned char *weatherIcon;

  // https://openweathermap.org/weather-conditions
  if (weatherConditionCode > 801) { // Cloudy
    weatherIcon = cloudy;
  } else if (weatherConditionCode == 801) { // Few Clouds
    weatherIcon = cloudsun;
  } else if (weatherConditionCode == 800) { // Clear
    weatherIcon = sunny;
  } else if (weatherConditionCode >= 700) { // Atmosphere
    weatherIcon = atmosphere;
  } else if (weatherConditionCode >= 600) { // Snow
    weatherIcon = snow;
  } else if (weatherConditionCode >= 500) { // Rain
    weatherIcon = rain;
  } else if (weatherConditionCode >= 300) { // Drizzle
    weatherIcon = drizzle;
  } else if (weatherConditionCode >= 200) { // Thunderstorm
    weatherIcon = thunderstorm;
  } else
    return;
  display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}
