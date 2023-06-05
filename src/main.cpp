#include "SunWatchy.h"
#include "settings.h"

SunWatchy watchy(settings);

void setup(){
  Serial.setDebugOutput(true);
  Serial.setRxBufferSize(1024);
  Serial.begin(115200);
  
  watchy.init();
}

void loop(){}
