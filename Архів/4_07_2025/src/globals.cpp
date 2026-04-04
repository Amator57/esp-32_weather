#include <TFT_eSPI.h>
#include "globals.h"
TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;
Adafruit_BME280 bme;
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


