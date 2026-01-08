#include <HTTPClient.h>
#include "time.h"
#include <string>
#include <ArduinoJson.h> 

#include "google.h"

const int maxEntryCount = 20;
struct calendarEntries calEnt[maxEntryCount];

char *calendarRequest = (char *) malloc(1024);

HTTPClient http;

int calEntryCount;
struct calendarEntries *getCalendar(String scriptId)
{
  sprintf(calendarRequest, "https://script.google.com/macros/s/%s/exec", scriptId.c_str());

  calEntryCount = 0;
  // Getting calendar from your published google script
  Serial.println("Getting calendar");
  Serial.println(calendarRequest);

  http.end();
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(calendarRequest)) {
    Serial.println("Cannot connect to google script");
    return calEnt;
  } 

  Serial.println("Connected to google script");
  int returnCode = http.GET();
  Serial.print("Returncode: "); Serial.println(returnCode);
  String response = http.getString();
  Serial.print("Response: "); Serial.println(response);

  int indexFrom = 0;
  int indexTo = 0;
  int cutTo = 0;

  String strBuffer = "";

  int count = 0;
  int line = 0;

  Serial.println("IndexFrom");  
  indexFrom = response.lastIndexOf("\n") + 1;

  // Fill calendarEntries with entries from the get-request
  while (indexTo>=0 && line<maxEntryCount) {
    count++;
    indexTo = response.indexOf(";",indexFrom);
    cutTo = indexTo;

    if(indexTo != -1) { 
      strBuffer = response.substring(indexFrom, cutTo);
      
      indexFrom = indexTo + 1;
      
      Serial.println(strBuffer);

      if(count == 1) {
        // Set entry time
        calEnt[line].calTime = strBuffer.substring(0,21);

      } else if(count == 2) {
        // Set entry title
        calEnt[line].calTitle = strBuffer;

      } else {
          count = 0;
          line++;
      }
    }
  }
  calEntryCount = line;
  return calEnt;
}