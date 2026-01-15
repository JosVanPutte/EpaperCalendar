// uncomment next line to use HSPI for EPD (and VSPI for SD), e.g. with Waveshare ESP32 Driver Board
#define LED_BUILTIN 2

#include <WiFiManager.h>
#include <time.h>
#include <rom/rtc.h>
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
/**
  struct tm wakeup;
  bool ok = getLocalTime(&wakeup);
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

*/
void goToSleep(int s) {
  uint64_t secondsToSleep = s;
  Serial.printf("Sleep for %d minutes.\n", secondsToSleep / 60);
  esp_sleep_enable_timer_wakeup(secondsToSleep * 1000000ULL); 
  esp_deep_sleep_start();
}

long bootCnt;
long totalTime;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  nvs_handle_t nvs = initNvs();
  Serial.println();
  Serial.println("setup");
  String bc = getNonVolatile(nvs, "bootCnt");
  String tc = getNonVolatile(nvs, "totalTime");
  if (bc.isEmpty() || tc.isEmpty()) {
    Serial.println("NVS values ??");
  } else {
    bootCnt = atol(bc.c_str());
    totalTime = atol(tc.c_str());
  }
  Serial.printf("bootCnt %ld total %ld s\n", bootCnt, totalTime / 1000);
  if (rtc_get_reset_reason(0) == POWERON_RESET) {
    Serial.println("POWER ON RESET");
    bootCnt = 0;
    totalTime = 0;
  }
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
  char buf[100];
  bootCnt++;
  totalTime += millis();
  sprintf(buf, "boot cnt %d total %ld s", bootCnt, totalTime / 1000);
  displayCalendar(buf, myCalendar);
  // go to sleep
  setNonVolatile(nvs, "bootCnt", bootCnt);
  setNonVolatile(nvs, "totalTime", totalTime);
  digitalWrite(2, LOW);
  goToSleep(60 * 60);
}

void loop() {
  // nothing here

}
