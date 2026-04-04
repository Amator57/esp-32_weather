#include <TFT_eSPI.h>
#include "globals.h"
TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;
Adafruit_BME280 bme;
int currentRotationIndex = 0;
DisplayMode currentDisplayMode = MODE_BME_DATA; // Або activeDisplayModes[0];

/*
const int screenW = 160;
const int screenH = 128;
const int lineHeight = 22;
*/
//const int screenW = 320;
//const int screenH = 240;
//const int lineHeight = 24;

int day = 1;  // початкове значення змінної
int  month=1;
int16_t year=2025;

unsigned char m=1;
unsigned char f_enter=0;
unsigned char f_plus=0;
unsigned char f_minus=0;
unsigned char f_long=0;
unsigned char week=0;
//Введена 08.07.2025 . файл main.cpp не запрацював і був повернений попередній.
std::vector<DisplayMode> activeDisplayModes = {
    MODE_BME_DATA,
    MODE_HISTORY_DATA,
    MODE_DATETIME,
    MODE_WIFI_INFO,
    MODE_TOLYA_VITA,
    MODE_UKRAINIAN,
    MODE_VIVAT,
    MODE_SLAVA_UKRAINI,
    MODE_SHEVCHENKO
};

