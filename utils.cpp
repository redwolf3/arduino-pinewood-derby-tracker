#include "Arduino.h"

void printTitle() {
  Serial.println("###########################################################");
  Serial.println("## Pinewoood Derby - Race Tracker - v0.1.0               ##");
  Serial.println("## Author: Steven West <redwolf3@steven-west.com>        ##");
  Serial.println("## Last Modified: 03/17/2024                             ##");
  Serial.println("###########################################################");
  Serial.println();
}

char *uptime(uint32_t millis) {
  static char _return[32];
  uint32_t secs=millis/1000, mins=secs/60;
  uint16_t hours=mins/60, days=hours/24;
  millis-=secs*1000;
  secs-=mins*60;
  mins-=hours*60;
  hours-=days*24;
  sprintf(_return,"Uptime %d days %2.2d:%2.2d:%2.2d.%3.3d", (uint8_t)days, (uint8_t)hours, (uint8_t)mins, (uint8_t)secs, (int16_t)millis);
  return _return;
}

char *elapsedTimeSecs(uint32_t elapsedMs) {
  static char _return[32];
  uint32_t secs=elapsedMs/1000;
  elapsedMs-=secs*1000;
  sprintf(_return,"%d.%3.3d", (uint16_t)secs, (int16_t)elapsedMs);
  return _return;
}