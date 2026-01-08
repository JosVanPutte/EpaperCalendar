#define USE_HSPI_FOR_EPD
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono18pt7b.h>
#include "display.h"

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_583_GDEQ0583T31
#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));

SPIClass hspi(HSPI);

int16_t tbx, tby;
uint16_t tbw, tbh;

void initDisplay(const char* initMessage, bool show)
{
  Serial.println("Initialize display");
  hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200);
  display.setRotation(0);
  display.setFont(&FreeMono18pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.getTextBounds(initMessage, 0, 0, &tbx, &tby, &tbw, &tbh);
  if (show) {
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.setFullWindow();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(x, y);
      display.print(initMessage);
    }  while (display.nextPage());
  }
  Serial.println("Initialize done");
}

void displayCalendar(struct calendarEntries *myCalendar) {
  uint16_t y = tbh;
  Serial.printf("font height %d calendar entries %d\n", tbh, calEntryCount);
  display.setFullWindow();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    for(int ntry = 0; ntry < calEntryCount; ntry++) {
      display.setFont(&FreeMonoBold18pt7b);
      display.setCursor(0, y);
      display.print(myCalendar[ntry].calTime);
      y += tbh + 2;
      display.setFont(&FreeMono18pt7b);
      display.setCursor(0, y);
      display.print(myCalendar[ntry].calTitle);
      y += tbh + 2;
    }
  }  while (display.nextPage());
}
