#ifndef SETTINGS_H
#define SETTINGS_H

//Weather Settings
#define CITY_ID "2938913" //Darmstdt https://openweathermap.org/current#2938913
#define OPENWEATHERMAP_APIKEY "41d21936d94cd9f21cd62ff368485d6a" //use your own API key :)
#define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?id=" //open weather api
#define TEMP_UNIT "metric" //metric = Celsius , imperial = Fahrenheit
#define TEMP_LANG "en"
#define WEATHER_UPDATE_INTERVAL 30 //must be greater than 5, measured in minutes
//NTP Settings
#define NTP_SERVER "de.pool.ntp.org"
#define GMT_OFFSET_SEC 3600 * 2 //New York is UTC -5 EST, -4 EDT, will be overwritten by weather data

watchySettings settings{
    .cityID = CITY_ID,
    .weatherAPIKey = OPENWEATHERMAP_APIKEY,
    .weatherURL = OPENWEATHERMAP_URL,
    .weatherUnit = TEMP_UNIT,
    .weatherLang = TEMP_LANG,
    .weatherUpdateInterval = WEATHER_UPDATE_INTERVAL,
    .ntpServer = NTP_SERVER,
    .gmtOffset = GMT_OFFSET_SEC,
    .vibrateOClock = false,
};

#endif
