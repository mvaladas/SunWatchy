#include "SunWatchy.h"
#include "Arduino.h"
#include "GxEPD2.h"
#include "freertos/portable.h"
#include "stdint.h"
#include <sunset.h>

#define DARKMODE false
#define DEG2RAD 0.0174532925
#define LAT 49.85959500649175 // Darmstadt
#define LON 8.639179730728287 // Darmstadt
#define CLOCK_RADIUS 75

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

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
                                 {1, 1, 1, 1},
                                 {1, 1, 1, 1},
                                 {1, 1, 1, 1},
                                 {0, 1, 1, 1},
                             }};

void SunWatchy::drawCircleSegment(int centerX, int centerY, int radius,
                                   double startAngle, double endAngle,
                                   int pattern) {
  int x, y;

  // Iterate through each pixel in the circle segment
  for (y = centerY - radius; y <= centerY + radius; y++) {
    for (x = centerX - radius; x <= centerX + radius; x++) {
      int dx = x - centerX;
      int dy = y - centerY;

      // Check if the pixel is within the circle segment
      int distanceSquared = dx * dx + dy * dy;
      double angle = atan2(dy, dx);

      if (distanceSquared <= radius * radius && angle >= startAngle &&
          angle <= endAngle) {

        int numRows =
            sizeof(grayPatterns[pattern]) / sizeof(grayPatterns[pattern][0]);
        int numCols = sizeof(grayPatterns[pattern][0]) /
                      sizeof(grayPatterns[pattern][0][0]);
        uint16_t color = grayPatterns[pattern][x % numRows][y % numCols] == 0
                             ? GxEPD_WHITE
                             : GxEPD_BLACK;
        display.drawPixel(x, y, color);
      }
    }
  }
}

void SunWatchy::drawTwilights() {
  sun.setPosition(LAT, LON, 2); // TODO fix this

  double civilsunrise;
  double civilsunset;
  double nauticalsunrise;
  double nauticalsunset;
  double astrosunrise;
  double astrosunset;
  double sunrise;
  double sunset;

  sun.setCurrentDate(currentTime.Year, currentTime.Month, currentTime.Day);

  sunrise = sun.calcSunrise();
  sunset = sun.calcSunset() - (12 * 60);
  civilsunrise = sun.calcCivilSunrise();
  civilsunset = sun.calcCivilSunset() - (12 * 60);
  nauticalsunrise = sun.calcNauticalSunrise();
  nauticalsunset = sun.calcNauticalSunset() - (12 * 60);
  astrosunrise = sun.calcAstronomicalSunrise();
  astrosunset = sun.calcAstronomicalSunset() - (12 * 60);

  float riseAng = (sunrise * 360 / (12.0 * 60.0));
  float setAng = (sunset * 360 / (12.0 * 60.0));
  float civilRiseAng = (civilsunrise * 360 / (12.0 * 60.0));
  float civilSetAng = (civilsunset * 360 / (12.0 * 60.0));
  float nauticalRiseAng = (nauticalsunrise * 360 / (12.0 * 60.0));
  float nauticalSetAng = (nauticalsunset * 360 / (12.0 * 60.0));
  float astroRiseAng = (astrosunrise * 360 / (12.0 * 60.0));
  float astroSetAng = (astrosunset * 360 / (12.0 * 60.0));

  if (riseAng > 180.0) {
    riseAng -= 360;
  }
  if (setAng > 180) {
    setAng -= 360;
  }
  if (civilRiseAng > 180.0) {
    civilRiseAng -= 360;
  }
  if (civilSetAng > 180) {
    civilSetAng -= 360;
  }
  if (nauticalRiseAng > 180.0) {
    nauticalRiseAng -= 360;
  }
  if (nauticalSetAng > 180) {
    nauticalSetAng -= 360;
  }
  if (astroRiseAng > 180.0) {
    astroRiseAng -= 360;
  }
  if (astroSetAng > 180) {
    astroSetAng -= 360;
  }
  drawCircleSegment(100, 100, CLOCK_RADIUS, setAng * DEG2RAD - PI / 2, 0, 0);
  drawCircleSegment(100, 100, CLOCK_RADIUS, 0, riseAng * DEG2RAD - PI / 2, 0);
  drawCircleSegment(100, 100, CLOCK_RADIUS, civilSetAng * DEG2RAD - PI / 2, 0,
                    1);
  drawCircleSegment(100, 100, CLOCK_RADIUS, 0, civilRiseAng * DEG2RAD - PI / 2,
                    1);
  drawCircleSegment(100, 100, CLOCK_RADIUS, nauticalSetAng * DEG2RAD - PI / 2,
                    0, 2);
  drawCircleSegment(100, 100, CLOCK_RADIUS, 0,
                    nauticalRiseAng * DEG2RAD - PI / 2, 2);
  if (isnan(astroRiseAng) || isnan(astroSetAng)) {
    drawCircleSegment(100, 100, CLOCK_RADIUS, astroSetAng * DEG2RAD - PI / 2, 0,
                      3);
    drawCircleSegment(100, 100, CLOCK_RADIUS, 0,
                      astroRiseAng * DEG2RAD - PI / 2, 3);
  }
}
void SunWatchy::drawWatchBorder() {
  for (int i = 0; i < 5; i++) {
    display.drawCircle(100, 100, CLOCK_RADIUS + i, GxEPD_BLACK);
  }
  display.fillRect(96, 20, 8, 16, GxEPD_BLACK);
  display.fillRect(96, 164, 8, 16, GxEPD_BLACK);

  display.fillRect(20, 96, 16, 8, GxEPD_BLACK);
  display.fillRect(164, 96, 16, 8, GxEPD_BLACK);

  for (int i = 0; i < 4; i++) {
    display.drawCircle(100, 100, CLOCK_RADIUS + i, GxEPD_BLACK);
  }
}

void SunWatchy::drawHands() {
  int handSize = 5;
  Coordinates t1, t2, t3, t4;
  float hour = ((currentTime.Hour + 11) % 12) + 1;
  float minutes = (currentTime.Minute + hour * 60.0);
  float angle = (minutes * 2 * PI / (12.0 * 60.0));

  t1.fromPolar(handSize, angle);
  t2.fromPolar(handSize, angle + PI);
  t3.fromPolar(CLOCK_RADIUS - 10, angle - PI / 2);
  t4.fromPolar(handSize * 4, angle + PI / 2);

  display.fillCircle(100, 100, handSize + 3, GxEPD_BLACK);
  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100,
                       t2.getY() + 100, t3.getX() + 100, t3.getY() + 100,
                       GxEPD_BLACK);

  display.fillTriangle(t1.getX() + 100, t1.getY() + 100, t2.getX() + 100,
                       t2.getY() + 100, t4.getX() + 100, t4.getY() + 100,
                       GxEPD_BLACK);
}

void SunWatchy::drawWatchFace() {
  display.fillScreen(GxEPD_WHITE);
  display.fillCircle(100, 100, CLOCK_RADIUS - 5, GxEPD_BLACK);
  display.fillCircle(100, 100, CLOCK_RADIUS, GxEPD_WHITE);
  drawTwilights();
  drawHands();
  drawWatchBorder();
  drawBattery();

  /* drawTime(); */
  drawDate();
  /* drawSteps(); */
  /* drawWeather(); */

  display.drawBitmap(130, 5, WIFI_CONFIGURED ? wifi : wifioff, 26, 18,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  if (BLE_CONFIGURED) {
    display.drawBitmap(110, 3, bluetooth, 13, 21,
                       DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  }
}

void SunWatchy::drawTime() {
  display.setTextColor(GxEPD_WHITE);
  /* display.setTextSize(1); */
  display.setFont(&DSEG7_Classic_Regular_15);
  display.setCursor(50, 53 + 5);
  int displayHour;
  if (HOUR_12_24 == 12) {
    displayHour = ((currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = currentTime.Hour;
  }
  if (displayHour < 10) {
    display.print("0");
  }
  display.print(displayHour);
  display.print(":");
  if (currentTime.Minute < 10) {
    display.print("0");
  }
  display.println(currentTime.Minute);
}

void SunWatchy::drawDate() {
  display.setFont(&Seven_Segment10pt7b);
  display.setTextColor(GxEPD_BLACK);

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
  daymon += ". " + month + ".\n";
  daymon += dayShortStr(currentTime.Wday);
  daymon += ".";
  display.getTextBounds(daymon, 5, 5, &x1, &y1, &w, &h);
  display.setCursor(5, 5 + h / 2);
  display.println(daymon);

  /* display.setCursor(5, 150); */
  /* display.println(tmYearToCalendar( */
  /*     currentTime.Year)); // offset from 1970, since year is stored in
   * uint8_t */
}
void SunWatchy::drawSteps() {
  // reset step counter at midnight
  if (currentTime.Hour == 0 && currentTime.Minute == 0) {
    sensor.resetStepCounter();
  }
  uint32_t stepCount = sensor.getCounter();
  display.drawBitmap(10, 165, steps, 19, 23,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  display.setCursor(35, 190);
  display.println(stepCount);
}
void SunWatchy::drawBattery() {
  display.drawBitmap(164, 1, battery, 37, 21,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  display.fillRect(169, 6, 27, BATTERY_SEGMENT_HEIGHT,
                   DARKMODE ? GxEPD_BLACK
                            : GxEPD_WHITE); // clear battery segments
  int8_t batteryLevel = 0;
  float VBAT = getBatteryVoltage();
  if (VBAT > 4.1) {
    batteryLevel = 3;
  } else if (VBAT > 3.95 && VBAT <= 4.1) {
    batteryLevel = 2;
  } else if (VBAT > 3.80 && VBAT <= 3.95) {
    batteryLevel = 1;
  } else if (VBAT <= 3.80) {
    batteryLevel = 0;
  }

  for (int8_t batterySegments = 0; batterySegments < batteryLevel;
       batterySegments++) {
    display.fillRect(169 + (batterySegments * BATTERY_SEGMENT_SPACING), 6,
                     BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  }
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
  display.drawBitmap(165, 110, currentWeather.isMetric ? celsius : fahrenheit,
                     26, 20, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
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
  display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH,
                     WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}
