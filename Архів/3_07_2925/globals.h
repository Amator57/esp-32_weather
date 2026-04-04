#pragma once
#include <vector> // Обов'язково додайте цей рядок на початку файлу globals.h для використання std::vector
#include <SPI.h>
#include "FS.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "SPIFFS.h"
#include <TFT_eSPI.h>
#include <Adafruit_BME280.h>
//#include "RTClib.h"


#include <RTClib.h>
//#include "text_effects.h"
// ==========================================================
// ОБОВ'ЯЗКОВО: Спочатку оголосіть сам перелік DisplayMode
// ==========================================================
enum DisplayMode {
  MODE_NONE,
  MODE_TOLYA_VITA,
  MODE_UKRAINIAN,
  MODE_VIVAT,
  MODE_SLAVA_UKRAINI,
  MODE_DATETIME,
  MODE_SHEVCHENKO,
  MODE_BME_DATA
};
extern RTC_DS3231 rtc;
extern Adafruit_BME280 bme;
// ==========================================================
// Потім оголошуйте всі глобальні змінні та функції, які використовують DisplayMode
// ==========================================================

extern DisplayMode currentDisplayMode;
extern unsigned long displayModeStartTime;

// TFT об'єкт
extern TFT_eSPI tft;
extern const int screenW;
extern const int screenH;
extern const int lineHeight;
// ==== Нові глобальні змінні для ротації дисплея ====
extern std::vector<DisplayMode> activeDisplayModes; // Список режимів, які зараз активні для ротації
extern int currentRotationIndex;                    // Індекс поточного сюжету у списку ротації
extern bool rotationActive;                         // Прапор, який показує, чи є що ротувати
// У globals.h
extern DisplayMode previousDisplayMode;
// Оголошення нових допоміжних функцій
void executeDisplayMode(DisplayMode mode);
unsigned long getDisplayDuration(DisplayMode mode);

// У globals.h
extern int lastDisplayedHour;
extern int lastDisplayedMinute;
extern int lastDisplayedDay;
extern int lastDisplayedMonth;
extern int lastDisplayedYear;
extern int lastDisplayedDayOfTheWeek; // 0=Sunday, 1=Monday...

// Ім'я .vlw-шрифту, який має бути в SPIFFS

//#define My_ariali_12 "ariali12"
//#define My_ariali_14 "ariali14"
//#define My_ariali_16 "ariali16"
//#define My_ariali_18 "ariali18"
#define My_ariali_20 "ariali20"
#define My_ariali_24 "ariali24"
#define My_ariali_26 "ariali26"
#define My_ariali_28 "ariali28"
#define My_ariali_30 "ariali30"
#define My_ariali_34 "ariali34"

//#define My_arialbi_28 "ariali34"
//#define My_arialbi_30 "ariali34"
//#define My_arialbi_34 "ariali34"
//#define My_arial_14 "arial14"
//#define My_arial_16 "arial16"

//#define My_arialitalic16 "arialitalic16"

#define TFT_MOSI   23
#define TFT_SCLK   18
#define TFT_CS      5
#define TFT_DC      2
#define TFT_RST     4
#define TOUCH_CS    3
//#define PIN_LED   6
#define UVOD       32
#define MINUS      33
#define PLUS       25


// У globals.h або text_data.h
#define DURATION_TOLYA_VITA    10000 // 10 секунд
#define DURATION_UKRAINIAN     10000
#define DURATION_VIVAT         10000
#define DURATION_SLAVA_UKRAINI 10000
#define DURATION_DATETIME      10000
#define DURATION_SHEVCHENKO    10000
#define DURATION_BME_DATA      10000
//bool button(int id);  // <-- додай це
extern int day;  // <-- додай це
extern int  month;
extern int16_t year;
#include "Set_year.h"
#include "Set_date.h"
#include "Set_month.h"
#include "Set_weekday.h"
#include "Set_hour.h"
#include "PUSH.h"
#include "week_day.h"
void week_day_out (unsigned char x);
void executeDisplayMode(DisplayMode mode);      // Оголошення нової функції
unsigned long getDisplayDuration(DisplayMode mode); // Оголошення нової функції

extern unsigned char m;
extern unsigned char f_enter;
extern unsigned char f_plus;
extern unsigned char f_minus;
extern unsigned char f_long;
extern unsigned char week;