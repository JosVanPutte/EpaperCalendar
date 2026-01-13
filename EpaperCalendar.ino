// uncomment next line to use HSPI for EPD (and VSPI for SD), e.g. with Waveshare ESP32 Driver Board
#define LED_BUILTIN 2

#include <WiFiManager.h>
#include <time.h>
#include "google.h"
#include "storage.h"
#include "display.h"

/**
define the google script ID here to access you script URL
sprintf(calendarRequest, "https://script.google.com/macros/s/%s/exec", GOOGLE_ID);
**/ 
//#define GOOGLE_ID "your google script ID"
#ifdef GOOGLE_ID
String googleId = GOOGLE_ID;
#else
String googleId;
#endif

const char setupWifi[] = "connect to wifi 'FridgeCalendar'";

char timeStr[50];
const char *monthName[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

/**
 * sync time with NTP
 */
void syncTime() {
  struct timeval tm;
  configTime(0, 0, "nl.pool.ntp.org");
  gettimeofday(&tm, NULL);
  settimeofday(&tm, NULL);
  struct tm wakeup;
  bool ok = getLocalTime(&wakeup);
  sprintf(timeStr, "wakeup on %d %s %d %02d:%02d:%02d", wakeup.tm_mday, monthName[wakeup.tm_mon], wakeup.tm_year % 100, wakeup.tm_hour, wakeup.tm_min, wakeup.tm_sec);
  Serial.println(timeStr);
}


void goToSleep() {
  struct tm wakeup;
  bool ok = getLocalTime(&wakeup);
  uint64_t secondsToSleep = 24 * 60 * 60;
  if (!ok) {
    Serial.printf("Sleep for %d minutes.\n", secondsToSleep / 60);
  } else {
    Serial.printf("up %d s time %02d:%02d:%02d\n", millis() / 1000, wakeup.tm_hour, wakeup.tm_min, wakeup.tm_sec);
    struct tm midnight = wakeup;
    midnight.tm_hour = 23;
    midnight.tm_min = 59;
    midnight.tm_sec = 59;
    Serial.printf("1 s for midnight day %d %02d:%02d:%02d\n", midnight.tm_mday, midnight.tm_hour, midnight.tm_min, midnight.tm_sec);
    time_t today = mktime(&wakeup);
    time_t tomorrow = mktime(&midnight);
    double timeDiff = difftime(tomorrow, today);
    secondsToSleep = trunc(timeDiff) + 1;
    Serial.printf("Midnight is %llu seconds away\n", secondsToSleep);
  }
  esp_sleep_enable_timer_wakeup(secondsToSleep * 1000000ULL); 
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  nvs_handle_t nvs = initNvs();
  Serial.println();
  Serial.println("setup");
#ifndef GOOGLE_ID
  googleId = getNonVolatile(nvs, "scriptId");
#endif
  initDisplay(setupWifi, googleId.isEmpty());
  WiFiManager wm;
  WiFi.mode(WIFI_STA);
  if (googleId.isEmpty()) {
    wm.resetSettings();
    WiFiManagerParameter googleIdPar = WiFiManagerParameter("scriptId", "script Id", "", 250);
    wm.addParameter(&googleIdPar);
    wm.autoConnect("FridgeCalendar");
    setNonVolatile(nvs,"scriptId", googleIdPar.getValue());
    googleId = googleIdPar.getValue();
  } else {
    wm.autoConnect("FridgeCalendar");
  }
  syncTime();
  // get the calendar
  struct calendarEntries *myCalendar = getCalendar(googleId);
  displayCalendar(timeStr, myCalendar);
  // go to sleep
  goToSleep();
}

void loop() {
  // nothing here

}
