#pragma once
#include <vector> // Обов'язково додайте цей рядок на початку файлу globals.h для використання std::vector
#include <SPI.h>
#include "FS.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "SPIFFS.h"
#include <TFT_eSPI.h>
#include <Adafruit_BME280.h>
#include <RTClib.h> 
#include <Preferences.h>      // Додайте, якщо Preferences використовується глобально
#include <ESPAsyncWebServer.h> // Додайте, якщо AsyncWebServer використовується глобально
#include <ArduinoJson.h>      // Додайте, якщо ArduinoJson використовується глобально

// Структура для збереження даних BME280
struct BMEData {
    float temperature;
    float humidity;
    float pressure;
    char timeStr[20];
};

extern uint32_t totalMeasurements;
extern uint32_t lastSavedTotal;

#define MAX_MEASUREMENTS 1440

// ==========================================================
// ОБОВ'ЯЗКОВО: Спочатку оголосіть сам перелік DisplayMode
// ==========================================================
// Визначення перерахування (тільки тут!)
enum DisplayMode {
    MODE_NONE = -1,
    MODE_BME_DATA,      // Поточні дані BME280
    MODE_HISTORY_DATA,  // Останні записи BME280 (історія)
    MODE_WIFI_INFO,     // Інформація про WiFi/IP
    MODE_DATETIME,      // Дата та час
    MODE_TOLYA_VITA,
    MODE_UKRAINIAN,
    MODE_VIVAT,
    MODE_SLAVA_UKRAINI,
    MODE_SHEVCHENKO,
    MODE_MAX // Завжди останній елемент, для підрахунку кількості режимів
};
/*
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
*/
extern RTC_DS3231 rtc;
extern Adafruit_BME280 bme;
extern Preferences preferences;  // Оголошення глобального об'єкта Preferences
extern AsyncWebServer server;    // Оголошення глобального об'єкта AsyncWebServer

// ==========================================================
// Потім оголошуйте всі глобальні змінні та функції, які використовують DisplayMode
// ==========================================================

extern DisplayMode currentDisplayMode;
extern DisplayMode previousDisplayMode; // Додано, якщо використовується для логіки перемикання режимів
extern unsigned long displayModeStartTime;
extern int currentRotationIndex;
extern bool rotationActive;
extern std::vector<DisplayMode> activeDisplayModes;
extern BMEData history[MAX_MEASUREMENTS];
extern int historyIndex;

// TFT об'єкт
extern TFT_eSPI tft;

// === НОВІ ОГОЛОШЕННЯ: Глобальні константи для розмірів екрану та висоти лінії ===
// Ці змінні повинні бути визначені в одному з .cpp файлів (наприклад, main.cpp)
// після ініціалізації tft, якщо їх значення залежать від tft.width()/tft.height().
// Якщо екран має фіксований розмір (наприклад, 240x320), їх можна визначити як const int.
extern const int screenW; // Ширина екрану
extern const int screenH; // Висота екрану
extern const int lineHeight; // Висота лінії тексту (якщо використовується глобально для відступів)


extern String deviceName;
extern String wifiSSID;
extern String wifiPassword;
extern String tzString;
extern bool useDST;
extern int tzOffset;
extern bool switch1, switch2, switch3, switch4, switch5, switch6, switch7; // Змінено, щоб відповідати main.cpp snippet (ініціалізація = false)
extern float lastTemp, lastHum, lastPress; // Змінено, щоб відповідати main.cpp snippet (ініціалізація = 0)
extern float tempOffset;
extern float humOffset;

/*
// === Оголошення змінних для відстеження останніх відображених значень (для MODE_DATETIME) ===
// Ці змінні використовуються для умовного оновлення екрану часу
extern int lastDisplayedHour;
extern int lastDisplayedMinute;
extern int lastDisplayedDay;
extern int lastDisplayedMonth;
extern int lastDisplayedYear;
extern int lastDisplayedDayOfTheWeek; 
*/

// Оголошення глобальних функцій (їх визначення мають бути у відповідних .cpp файлах, наприклад, main.cpp або subroutines.cpp)
void executeDisplayMode(DisplayMode mode);
void displayDateTime();
void week_day_out(unsigned char x);
unsigned long getDisplayDuration(DisplayMode mode); // Додайте, якщо ця функція є глобальною і використовується поза файлом її визначення
void printLineTypingEffect(const char* text, int screenWidth, int lineHeight); // Додано, оскільки використовується в shevchenko/vivat/slava_ukraini
void displayBMEData(); // <--- ДОДАЙТЕ ЦЕЙ РЯДОК
String getTodayDate(); // Повертає дату без зміщення


// === Оголошення функцій для кнопок, які викликали помилки "not declared in this scope" ===
//bool button(int pin);      // Оголошення функції читання стану кнопки
//bool long_button(int pin); // Оголошення функції для довгого натискання кнопки
//bool uvod();               // Оголошення функції для кнопки "Введення" (можливо, UVOD)


// Макроси для шрифтів
#define My_ariali_26 "ariali26"
#define My_ariali_24 "ariali24"
#define My_ariali_28 "ariali28"
#define My_ariali_30 "ariali30"
#define My_ariali_34 "ariali34"
// === НОВЕ ОГОЛОШЕННЯ: ШРИФТ, ЯКИЙ БУВ NOT DECLARED IN THIS SCOPE ===
#define My_ariali_20 "ariali20" // Додано

// Піни (якщо вони використовуються глобально)
// TFT піни тепер визначені в platformio.ini через build_flags
#define UVOD       32
#define MINUS      33
#define PLUS       25

// Тривалість відображення режимів
#define DURATION_TOLYA_VITA    10000 // 10 секунд
#define DURATION_UKRAINIAN     10000
#define DURATION_VIVAT         10000
#define DURATION_SLAVA_UKRAINI 10000
#define DURATION_DATETIME      10000
#define DURATION_SHEVCHENKO    10000
#define DURATION_BME_DATA      10000

//#endif // GLOBALS_H