#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <Wire.h>
#include <RTClib.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h> //
#include <freertos/task.h> //

#include "Tolya_Vita320_240.h"
#include "ukrainian320_240.h"

#include "globals.h"
#include "text_data.h"
#include "draw_utils.h"
#include "subroutines.h"


AsyncWebServer server(80);
Preferences preferences;

String deviceName = "ESP32_Weather";
String wifiSSID = "Unknown";
String tzString = "EET-2EEST,M3.5.0/3,M10.5.0/4";

bool useDST = true;       // Літній час
int tzOffset = 2;  // за замовчуванням для Києва (UTC+2)
// Ініціалізація глобальних змінних для ротації дисплея
std::vector<DisplayMode> activeDisplayModes;
int currentRotationIndex = 0;
bool rotationActive = false;
// У main.cpp або subroutines.cpp
DisplayMode previousDisplayMode = MODE_NONE; // Ініціалізуємо попередній режим відображення
// У main.cpp або subroutines.cpp
int lastDisplayedHour = -1; // Ініціалізуємо -1, щоб забезпечити перше відображення
int lastDisplayedMinute = -1;
int lastDisplayedDay = -1;
int lastDisplayedMonth = -1;
int lastDisplayedYear = -1;
int lastDisplayedDayOfTheWeek = -1;

// У main.cpp
// ... після extern TFT_eSPI tft;
// ... або після інших глобальних змінних

const int screenW = 320; // Встановіть тут фактичну ширину вашого екрану
const int screenH = 240; // Встановіть тут фактичну висоту вашого екрану
const int lineHeight = 28; // Встановіть бажану висоту лінії для тексту, наприклад, 28-30px для шрифту 26pt. Можете використовувати tft.fontHeight() + відступ.

// ... решта ваших глобальних змінних

bool switch1 = false, switch2 = false, switch3 = false, switch4 = false;
bool switch5 = false, switch6 = false, switch7 = false;
float lastTemp = 0, lastHum = 0, lastPress = 0;
float temperature;
float pressure;
float humidity;
int8_t   hour, minute, second;
long x;//усереднення тиску
//unsigned char i;//усереднення тиску
//unsigned char h;//змінна вологості
//unsigned char q = 0;//змінна входження в режим встановлення
//unsigned char v = 0;
//unsigned char s = 0;

unsigned char flag_s=1;
unsigned char flag_w=1;
unsigned char  ind;
#define MAX_POINTS 1440
#define MAX_MEASUREMENTS 1440

struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

BMEData history[MAX_MEASUREMENTS];
int historyIndex = 0;
//+++++++++++++++=============
// Додайте ці оголошення в globals.h або на початку main.cpp
/* перенесено в globals.h
enum DisplayMode {
  MODE_NONE, // Жоден специфічний режим відображення не активний (можна показувати фон або дефолтний екран)
  MODE_TOLYA_VITA,
  MODE_UKRAINIAN,
  MODE_VIVAT,
  MODE_SLAVA_UKRAINI,
  MODE_DATETIME,
  MODE_SHEVCHENKO,
  MODE_BME_DATA
};
*/
DisplayMode currentDisplayMode = MODE_NONE;
unsigned long displayModeStartTime = 0; // Час початку поточного режиму відображення
/*
// Визначте тривалість для кожного режиму в мілісекундах
const unsigned long DURATION_TOLYA_VITA = 10000;      // 10 секунд
const unsigned long DURATION_UKRAINIAN = 10000;      // 10 секунд
const unsigned long DURATION_VIVAT = 5000;           // 5 секунд
const unsigned long DURATION_SLAVA_UKRAINI = 3000;   // 3 секунди (налаштуйте за потребою)
const unsigned long DURATION_DATETIME = 10000;       // 10 секунд
const unsigned long DURATION_SHEVCHENKO = 5000;      // 5 секунд
const unsigned long DURATION_BME_DATA = 10000;       // 10 секунд
*/
// Оголошення нових допоміжних функцій для відображення
void displayTolyaVita();
void displayUkrainian();
void displayVivat();
void displaySlavaUkraini();
void displayDateTime();
void displayShevchenko();
void displayBMEData();
void displayDefaultBackground();
void saveHistoryTask();
void performHistoryFileSave();// Оголошення нової функції для збереження історії

void clearOldLogs(int days); // Оголошення для функції очищення старих логів
void clearHistoryData();     // Оголошення для функції очищення масиву поточних даних
// Допоміжні функції для відображення на TFT

void displayTolyaVita() {
Serial.println("displayTolyaVita() викликано. Спроба виведення зображення TolyaVita."); // Додано для налагодження
 // tft.unloadFont(); // Додайте це!
  tft.fillScreen(TFT_BLACK);
  
  tft.pushImage(0, 0, 320, 240, IMG_Tolya_Vita320_240);
}

void displayUkrainian() {
  tft.unloadFont(); // Додайте це!
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(0, 0, 320, 240, IMG_ukrainian320_240);
}

void displayVivat() {
  tft.fillScreen(TFT_BLACK);
 // tft.loadFont(My_ariali_26);
  vivat(); // Ваша функція для відображення "Vivat"
  //tft.unloadFont();
}

void displaySlavaUkraini() {
  tft.fillScreen(TFT_BLACK);
  //tft.loadFont(My_ariali_34);
  slava_ukraini(); // Ваша функція для відображення "Slava Ukraini"
  //tft.unloadFont();
}
/////////////
void displayDateTime() {
    tft.fillScreen(TFT_BLACK); // Повністю очищаємо екран

    tft.setTextColor(TFT_GREEN, TFT_BLACK); // Встановлюємо колір тексту (білий на чорному)
    tft.loadFont(My_ariali_34); // Завантажуємо шрифт. Переконайтесь, що він завантажується тут або в setup.

    DateTime now = rtc.now(); // Отримуємо поточний час

    // Відображення часу (години:хвилини)
    tft.setCursor(100, 50); // Встановлюємо позицію для часу
    tft.printf("%02d:%02d", now.hour(), now.minute());
    tft.setTextColor(TFT_RED, TFT_BLACK); // Встановлюємо колір тексту (білий на чорному)
    // Відображення дати (день.місяць.рік)
    tft.setCursor(60, 110); // Встановлюємо позицію для дати
    tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());

    // Відображення дня тижня
    // Припускаємо, що week_day_out() також використовує глобальний об'єкт tft
    week_day_out(now.dayOfTheWeek());
}
/*
// У вашому файлі subroutines.cpp або main.cpp
void displayDateTime() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Колір тексту для часу та дати

  DateTime now = rtc.now(); // Отримуємо поточний час з RTC

  // Перевіряємо, чи змінилися значення
  bool hourChanged = (now.hour() != lastDisplayedHour);
  bool minuteChanged = (now.minute() != lastDisplayedMinute);
  bool dayChanged = (now.day() != lastDisplayedDay);
  bool monthChanged = (now.month() != lastDisplayedMonth);
  bool yearChanged = (now.year() != lastDisplayedYear);
  bool dayOfTheWeekChanged = (now.dayOfTheWeek() != lastDisplayedDayOfTheWeek);

  // === Оновлення часу (HH:MM) ===
  // Завжди оновлюємо при першому запуску (lastDisplayedHour/Minute == -1)
  // Або коли змінилися години чи хвилини
  if (lastDisplayedHour == -1 || hourChanged || minuteChanged) {
    tft.fillRect(60, 50, 100, 30, TFT_BLACK); // Очищуємо область часу
    tft.setCursor(60, 50);
    tft.printf("%02d:%02d", now.hour(), now.minute()); // Вивід без секунд

    // Оновлюємо збережені значення
    lastDisplayedHour = now.hour();
    lastDisplayedMinute = now.minute();
  }

  // === Оновлення дати (DD.MM.YYYY) ===
  // Завжди оновлюємо при першому запуску (lastDisplayedDay == -1)
  // Або коли змінилися день, місяць або рік
  if (lastDisplayedDay == -1 || dayChanged || monthChanged || yearChanged) {
    tft.fillRect(60, 90, 120, 30, TFT_BLACK); // Очищуємо область дати
    tft.setCursor(60, 90);
    tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());

    // Оновлюємо збережені значення
    lastDisplayedDay = now.day();
    lastDisplayedMonth = now.month();
    lastDisplayedYear = now.year();
  }

  // === Оновлення дня тижня ===
  // Завжди оновлюємо при першому запуску (lastDisplayedDayOfTheWeek == -1)
  // Або коли змінився день тижня
  if (lastDisplayedDayOfTheWeek == -1 || dayOfTheWeekChanged) {
    week_day_out(now.dayOfTheWeek()); // Ця функція має власне очищення області
    lastDisplayedDayOfTheWeek = now.dayOfTheWeek();
  }
}
*/
/*
void displayDateTime() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Колір тексту для часу та дати

  DateTime now = rtc.now(); // Отримуємо поточний час з RTC

  // === Повністю перемальовуємо час (HH:MM) незалежно від змін ===
  tft.fillRect(60, 50, 100, 30, TFT_BLACK); // Очищуємо область часу
  tft.setCursor(60, 50);
  tft.printf("%02d:%02d", now.hour(), now.minute()); // Вивід без секунд

  // === Повністю перемальовуємо дату (DD.MM.YYYY) незалежно від змін ===
  tft.fillRect(60, 90, 120, 30, TFT_BLACK); // Очищуємо область дати
  tft.setCursor(60, 90);
  tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());

  // === Повністю перемальовуємо день тижня незалежно від змін ===
  week_day_out(now.dayOfTheWeek()); // Ця функція має власне очищення області
}
*/
/*
// Реалізація displayDateTime() (наприклад, у subroutines.cpp або main.cpp)
void displayDateTime() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Колір тексту для часу та дати

  DateTime now = rtc.now(); // Отримуємо поточний час з RTC

  // Перевірка, чи змінилися значення
  bool hourChanged = (now.hour() != lastDisplayedHour);
  bool minuteChanged = (now.minute() != lastDisplayedMinute);
  bool dayChanged = (now.day() != lastDisplayedDay);
  bool monthChanged = (now.month() != lastDisplayedMonth);
  bool yearChanged = (now.year() != lastDisplayedYear);
  bool dayOfTheWeekChanged = (now.dayOfTheWeek() != lastDisplayedDayOfTheWeek);

  // ==== Оновлення часу (HH:MM) ====
  // Перемальовуємо, тільки якщо змінилися години або хвилини
  if (hourChanged || minuteChanged) {
    tft.fillRect(60, 50, 100, 30, TFT_BLACK); // Очищуємо область часу
    tft.setCursor(60, 50);
    tft.printf("%02d:%02d", now.hour(), now.minute()); // Вивід без секунд

    // Оновлюємо збережені значення
    lastDisplayedHour = now.hour();
    lastDisplayedMinute = now.minute();
  }

  // ==== Оновлення дати (DD.MM.YYYY) ====
  // Перемальовуємо, тільки якщо змінилися день, місяць або рік
  if (dayChanged || monthChanged || yearChanged) {
    tft.fillRect(60, 90, 120, 30, TFT_BLACK); // Очищуємо область дати
    tft.setCursor(60, 90);
    tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());

    // Оновлюємо збережені значення
    lastDisplayedDay = now.day();
    lastDisplayedMonth = now.month();
    lastDisplayedYear = now.year();
  }

  // ==== Оновлення дня тижня ====
  // Перемальовуємо, тільки якщо змінився день тижня
  if (dayOfTheWeekChanged) {
    // week_day_out() вже містить tft.fillRect та tft.setTextColor
    week_day_out(now.dayOfTheWeek());

    // Оновлюємо збережене значення
    lastDisplayedDayOfTheWeek = now.dayOfTheWeek();
  }
}
*/
/*
// Реалізація displayDateTime() (наприклад, у subroutines.cpp або main.cpp)
void displayDateTime() {
  // Шрифт вже завантажений функцією executeDisplayMode(), коли MODE_DATETIME був активований.
  // === ВАЖЛИВО: Видаліть tft.loadFont() та tft.unloadFont() звідси! ===

  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Колір тексту для часу та дати

  DateTime now = rtc.now(); // Отримуємо поточний час з RTC

  // ==== Оновлення часу (HH:MM) ====
  tft.fillRect(60, 50, 100, 30, TFT_BLACK); // Очищуємо область часу
  tft.setCursor(60, 50);
  tft.printf("%02d:%02d", now.hour(), now.minute()); // Вивід без секунд

  // ==== Оновлення дати (DD.MM.YYYY) ====
  tft.fillRect(60, 90, 120, 30, TFT_BLACK); // Очищуємо область дати
  tft.setCursor(60, 90);
  tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());

  // ==== Оновлення дня тижня ====
  week_day_out(now.dayOfTheWeek()); // Ця функція також не повинна завантажувати/вивантажувати шрифт
}
*/
/*
// Реалізація displayDateTime() (наприклад, у subroutines.cpp або main.cpp)
void displayDateTime() {
  // === ВАЖЛИВО: Видаліть tft.fillScreen(TFT_BLACK); звідси! ===
  // Екран буде очищено функцією executeDisplayMode() один раз при вході в MODE_DATETIME.

  // Завантажуємо шрифт лише один раз для всіх елементів дати/часу/дня тижня
  tft.loadFont(My_ariali_26); // Припускаємо, що це шрифт, який ви використовуєте

  // Встановлюємо стандартний колір тексту для часу та дати
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 

  DateTime now = rtc.now(); // Отримуємо поточний час з RTC

  // ==== Оновлення часу (HH:MM:SS) ====
  // Спочатку очищуємо область, де був попередній час, щоб уникнути артефактів.
  // Розміри прямокутника (x, y, ширина, висота) потрібно адаптувати під ваш шрифт та позицію.
  // Для шрифту Arial 26, висота символу приблизно 26-30 пікселів.
  // Час "HH:MM:SS" - 8 символів. Припустимо, ширина 100 пікселів достатньо.
  tft.fillRect(60, 50, 100, 30, TFT_BLACK); // Позиція (60,50) - приклад, можливо, потрібно скоригувати
  tft.setCursor(60, 50);
 // tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  tft.printf("%02d:%02d", now.hour(), now.minute());
  // ==== Оновлення дати (DD.MM.YYYY) ====
  // Аналогічно, очищуємо область для дати.
  // Дата "DD.MM.YYYY" - 10 символів. Припустимо, ширина 120 пікселів.
  tft.fillRect(60, 90, 120, 30, TFT_BLACK); // Позиція (60,90) - приклад
  tft.setCursor(60, 90);
  tft.printf("%02d.%02d.%d", now.day(), now.month(), now.year());
  
  // ==== Оновлення дня тижня ====
  // Викликаємо week_day_out. Вона тепер сама буде керувати своїм очищенням та кольором.
  week_day_out(now.dayOfTheWeek());

  tft.unloadFont(); // Вивантажуємо шрифт після завершення всіх операцій відображення у цій функції
}
*/
////////////
/*
void displayDateTime() {
  tft.fillScreen(TFT_BLACK); // Очищаємо екран перед відображенням часу
  tft.loadFont(My_ariali_26);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  DateTime now = rtc.now();
  int currentDay = now.day();
  int currentMonth = now.month();
  int currentYear = now.year();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentWeekDay = now.dayOfTheWeek();

  tft.setCursor(80, 30);
  tft.printf("%02d:%02d", currentHour, currentMinute); // Форматований вивід годин і хвилин

  tft.setCursor(40, 80);
  tft.printf("%02d:%02d:%d", currentDay, currentMonth, currentYear); // Форматований вивід дати

  week_day_out(currentWeekDay); // Ваша функція для відображення дня тижня
  tft.unloadFont();
}
*/
void displayShevchenko() {
  tft.fillScreen(TFT_BLACK);
  //tft.loadFont(My_ariali_20);
  shevchenko(); // Ваша функція для відображення Шевченка
  //tft.unloadFont();
}

void displayBMEData() {
  
  tft.fillScreen(TFT_BLACK); // Очищаємо екран перед відображенням даних BME
  tft.loadFont(My_ariali_24);
   temperature = lastTemp;
    float avgPressure = lastPress;
    humidity = lastHum;
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Атмосферний тиск:", 30, 20);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  //avgPressure = (float)x / 1.0; // Використовуємо глобальну 'x'
  tft.setCursor(50, 50);
  tft.print(avgPressure / 133.322);
  tft.drawString(" mmHg", 160, 50);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(50, 80);
  tft.print(avgPressure);
  tft.drawString(" Pa", 160, 80);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Температура: ", 20, 110);
  tft.setCursor(190, 110);
  tft.print(temperature); // Глобальна змінна 'temperature'
  tft.drawString(" C", 250, 110);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Вологість:", 20, 170);
  tft.setCursor(155, 170);
  tft.print(humidity); // Глобальна змінна 'humidity'
  tft.drawString(" %", 210, 170);
  tft.unloadFont();
}

void displayDefaultBackground() {
  tft.setRotation(1); // Встановлюйте ротацію лише один раз в setup, якщо вона постійна
  tft.setTextWrap(false); // Встановлюйте лише один раз в setup
  tft.fillScreen(TFT_BLACK); // Очищаємо екран перед фоном
  tft.fillRect(0, 0, tft.width(), tft.height() / 2, TFT_BLUE);
  tft.fillRect(0, tft.height() / 2, tft.width(), tft.height() / 2, TFT_YELLOW);
}

//++++++++++++++==============
// --- Forward declarations (kept from original) ---
String getTodayDate();
void clearOldLogs(int maxDays);
// saveHistoryToFile() is replaced by saveHistoryTask()


//////////////
String buildTZString(int offsetHours, bool useDST) {
  String sign = offsetHours >= 0 ? "-" : "+";
  int absOffset = abs(offsetHours);
  String tz = "GMT" + sign + String(absOffset);
  if (useDST) tz += "DST,M3.5.0/3,M10.5.0/4";
  return tz;
}

///////////////Побудова графіків

void storeToHistory(float temp, float hum, float pres, String timeStr) {
  history[historyIndex] = { temp, hum, pres, timeStr };
  historyIndex = (historyIndex + 1) % MAX_POINTS;
}

//////////////
// ОНОВЛЕНО: Формат дати/часу на YYYY-MM-DD HH:MM:SS для кращої сумісності
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    DateTime now = rtc.now();
    char buf[20];
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    return String(buf);
  }
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

String processor(const String& var) {
  if (var == "CURRENT_TIME") {
    return getCurrentTime();  // функція формує поточний час
  }
  if (var == "DEVICE_NAME") return deviceName;
  if (var == "IP_ADDRESS") return WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  if (var == "WIFI_SSID") return wifiSSID;
  if (var == "TZ_OFFSET") return String(tzOffset);  // нова змінна: часовий зсув UTC, напр. -2
  if (var == "USE_DST_CHECKED") return useDST ? "checked" : "";
  if (var == "TEMP") return String(bme.readTemperature(), 1);
  if (var == "HUM") return String(bme.readHumidity(), 0);
  if (var == "PRES") return String(bme.readPressure() / 100.0F, 0);
  if (var == "SW1_CHECKED") return switch1 ? "checked" : "";
  if (var == "SW2_CHECKED") return switch2 ? "checked" : "";
  if (var == "SW3_CHECKED") return switch3 ? "checked" : "";
  if (var == "SW4_CHECKED") return switch4 ? "checked" : "";
  if (var == "SW5_CHECKED") return switch5 ? "checked" : "";
  if (var == "SW6_CHECKED") return switch6 ? "checked" : "";
  if (var == "SW7_CHECKED") return switch7 ? "checked" : "";
  return String();
}



void saveConfig() {
  preferences.begin("config", false);
  preferences.putString("deviceName", deviceName);
  preferences.putString("wifiSSID", wifiSSID);
  preferences.putInt("tzOffset", tzOffset);
  preferences.putBool("useDST", useDST);
  preferences.putBool("switch1", switch1);
  preferences.putBool("switch2", switch2);
  preferences.putBool("switch3", switch3);
  preferences.putBool("switch4", switch4);
  preferences.putBool("switch5", switch5);
  preferences.putBool("switch6", switch6);
  preferences.putBool("switch7", switch7);
  preferences.end();
}

void loadConfig() {
  preferences.begin("config", true);
  deviceName = preferences.getString("deviceName", deviceName);
  wifiSSID = preferences.getString("wifiSSID", wifiSSID);
  tzOffset = preferences.getInt("tzOffset", 2);
  useDST = preferences.getBool("useDST", true);
  switch1 = preferences.getBool("switch1", false);
  switch2 = preferences.getBool("switch2", false);
  switch3 = preferences.getBool("switch3", false);
  switch4 = preferences.getBool("switch4", false);
  switch5 = preferences.getBool("switch5", false);
  switch6 = preferences.getBool("switch6", false);
  switch7 = preferences.getBool("switch7", false);
  preferences.end();

  // Встановлення TZ після завантаження
  String sign = tzOffset >= 0 ? "-" : "+";
  int absOffset = abs(tzOffset);
  String tzString = "UTC" + sign + String(absOffset);
  if (useDST) {
    tzString += "DST,M3.5.0/3,M10.5.0/4";
  }
  setenv("TZ", tzString.c_str(), 1);
  tzset();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// --- Отримати сьогоднішню дату у форматі РРРР-ММ-ДД ---
String getTodayDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "unknown";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}

// --- NEW FreeRTOS Task for saving history ---
// Замінює стару функцію saveHistoryToFile()
void saveHistoryTask(void * parameter) {
  Serial.println("Початок фонового збереження історії...");

  String filename = String("/log/") + getTodayDate() + ".csv";

  // Створюємо теку /log, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);  // Перезапис
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    vTaskDelete(NULL); // Завершити завдання, якщо не вдалося відкрити файл
    return;
  }

  file.write(0xEF); file.write(0xBB); file.write(0xBF); // UTF-8 BOM
  file.println("Time,Temperature,Humidity,Pressure");  // Заголовок CSV

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (i % 50 == 0) {
      yield();
    }

    if (d.timeStr == "") {
      continue;  // пропустити порожні записи
    }

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Фонове архівування завершено: %s (%d записів)\n", filename.c_str(), written);

  vTaskDelete(NULL); // Завершити завдання після його виконання
}

// ==== Реалізація функції clearHistoryData() ====
// Розмістіть цей блок коду в одному з ваших .cpp файлів (наприклад, main.cpp або subroutines.cpp).
// Ця функція очищує масив history, обнуляючи всі його записи.
void clearHistoryData() {
  // Проходимося по кожному елементу масиву history
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    // Обнуляємо поля кожного запису.
    // Припускається, що BMEData є структурою/класом, визначеною у вас
    // і може бути ініціалізована таким чином.
    // Якщо BMEData має інший конструктор за замовчуванням, використовуйте його.
    history[i] = {0.0F, 0.0F, 0.0F, ""};
  }
  // Скидаємо індекс історії на 0, щоб записи починалися спочатку
  historyIndex = 0;
  Serial.println("✅ History data cleared for new day's logging.");
}

// --- Видалити лог-файли старші за N днів ---
void clearOldLogs(int maxDays) {
  Serial.printf("DEBUG: Clearing logs older than %d days (real implementation needed).\n", maxDays);
  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити теку /log або вона не є директорією.");
    return;
  }

  time_t now;
  time(&now);

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    file.close(); // Закрити файл перед видаленням, якщо потрібно

    // Перевіряємо, чи це CSV-файл і чи він у теці /log
    if (name.startsWith("/log/") && name.endsWith(".csv")) {
      String fileNameOnly = name;
      fileNameOnly.replace("/log/", ""); // Отримуємо лише ім'я файлу, напр. "2025-06-28.csv"

      struct tm tm = {};
      // Парсимо дату з імені файлу (формат РРРР-MM-DD.csv)
      if (strptime(fileNameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
        time_t fileTime = mktime(&tm);
        double ageDays = difftime(now, fileTime) / 86400.0; // Різниця в днях

        if (ageDays > maxDays) {
          if (SPIFFS.remove(name)) { // Видаляємо файл
            Serial.println("[DEL] Старий файл видалено: " + name);
          } else {
            Serial.println("❌ Не вдалося видалити старий файл: " + name);
          }
        }
      } else {
        Serial.println("⚠️ Не вдалося розпарсити дату з імені файлу: " + name);
      }
    }
    file = root.openNextFile(); // Переходимо до наступного файлу
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
 tft.init();
  tft.setRotation(1);   // Задати альбомну орієнтацію один раз
  tft.setTextWrap(false); // Задати обгортання тексту один раз
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);


  Wire.begin();
  rtc.begin();
  SPIFFS.begin(true);
  bme.begin(0x76);

  // ІНІЦІАЛІЗАЦІЯ МАСИВУ ІСТОРІЇ
  for (int j = 0; j < MAX_MEASUREMENTS; ++j) {
      history[j].timeStr = "";
      history[j].temperature = 0.0f;
      history[j].humidity = 0.0f;
      history[j].pressure = 0.0f;
  }

  loadConfig();
  ///////////////////
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();
////////////////////////
  WiFi.begin("House", "1234567890");
  delay(3000); // Даємо час для спроби підключення до STA

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Режим STA не підключився, запускаю режим AP...");
    // FIX: Виклик WiFi.softAPIP() переміщено після успішного старту WiFi.softAP()
    if (WiFi.softAP("ESP32_Config", "12345678")) {
      String apIP = WiFi.softAPIP().toString(); // Отримуємо IP тільки після старту AP
      Serial.println("✅ Режим AP успішно запущено. IP: " + apIP);
      tft.drawString("AP Mode: " + apIP, 10, 50);
    } else {
      Serial.println("❌ Не вдалося запустити режим AP!");
      tft.drawString("AP Mode Failed!", 10, 50);
    }
  } else {
    wifiSSID = WiFi.SSID();
    String ip = WiFi.localIP().toString();
    Serial.println("✅ Підключено до WiFi: " + wifiSSID + ", IP: " + ip);
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip, 10, 70);
  }
// === НОВЕ: Перше зчитування даних BME280 одразу після запуску ===
    // Цей блок коду заповнює lastTemp, lastHum, lastPress актуальними значеннями.
    lastTemp = bme.readTemperature();
    lastHum = bme.readHumidity();
    lastPress = bme.readPressure(); // lastPress зберігається в Паскалях

    Serial.printf("Initial BME280: Temp=%.1fC, Hum=%.1f%%, Press=%.1fPa\n", lastTemp, lastHum, lastPress);

    //
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // setenv("TZ", tzString.c_str(), 1); // Це вже встановлено в loadConfig та вище, дублювання не потрібне
  // tzset(); // Це вже встановлено в loadConfig та вище, дублювання не потрібне

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/graph.js", "application/javascript");
  });

  //////////////////
  server.on("/log_export", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("date")) {
      request->send(400, "text/plain", "❌ Вкажи параметр ?date=YYYY-MM-DD");
      return;
    }

    String date = request->getParam("date")->value(); // напр. "2025-06-28"
    String path = "/log/" + date + ".csv";

    if (!SPIFFS.exists(path)) {
      request->send(404, "text/html; charset=utf-8", "<h2>❌ Файл не знайдено</h2>");
      return;
    }

    // Забезпечуємо правильне завантаження файлу CSV
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, "text/csv");
    response->addHeader("Content-Disposition", "attachment; filename=\"" + date + ".csv\"");
    request->send(response);
  });

  /////////////////

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });
  //////////////////
  
  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", R"rawliteral(
      <html>
      <head>
        <meta charset="UTF-8">
        <title>Перезавантаження</title>
        <script>
          setTimeout(function() {
            window.location.href = "/";
          }, 8000); // почекай 8 секунд і перенаправ
        </script>
      </head>
      <body style="font-family: sans-serif; text-align: center; padding-top: 50px;">
        <h2>🔁 Пристрій перезавантажується...</h2>
        <p>Будь ласка, зачекайте кілька секунд.</p>
      </body>
      </html>
    )rawliteral");
    delay(500); // дати час відповіді HTTP
    ESP.restart();
  });


  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.on("/bme.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bme.html", "text/html; charset=utf-8");
  });



  server.on("/bme_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = bme.readTemperature();
    doc["humidity"]    = bme.readHumidity();
    doc["pressure"]    = bme.readPressure() / 100.0F;
    doc["presmmhg"]    = bme.readPressure() / 133.322F;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });


  // ==== Графік поточних даних (динамічна вісь Y) ==== 
  server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");
    bool first = true;

    // Змінні для динамічного діапазону графіку
    float minTemp = 1000, maxTemp = -1000;
    float minHum = 1000, maxHum = -1000;
    float minPress = 2000, maxPress = 0;

    for (int i = 0; i < MAX_POINTS; i++) {
      int idx = (historyIndex + i) % MAX_POINTS;
      const BMEData &d = history[idx];
      if (d.timeStr == "") continue;

      if (!first) response->print(",");
      first = false;

      // оновлення діапазонів
      if (d.temperature < minTemp) minTemp = d.temperature;
      if (d.temperature > maxTemp) maxTemp = d.temperature;
      if (d.humidity < minHum) minHum = d.humidity;
      if (d.humidity > maxHum) maxHum = d.humidity;
      if (d.pressure < minPress) minPress = d.pressure;
      if (d.pressure > maxPress) maxPress = d.pressure;

      response->printf(
        "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        d.timeStr.c_str(), d.temperature, d.humidity, d.pressure
      );
    }

    response->print("]");
    request->send(response);
  });


  //+++++++++++++++
  server.on("/log_list.json", HTTP_GET, [](AsyncWebServerRequest *request){
    File root = SPIFFS.open("/log");
    File file = root.openNextFile();

    String json = "[";
    bool first = true;

    while (file) {
      String path = file.name();
      if (!path.endsWith(".csv")) {
        file = root.openNextFile();
        continue;
      }
      if (!first) json += ",";
      first = false;

      String name = path;
      name.replace("/log/", "");
      json += "{\"name\":\"" + name + "\",\"path\":\"" + path + "\"}";
      file = root.openNextFile();
    }

    json += "]";
    request->send(200, "application/json", json);
  });

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<h2>Доступні архіви:</h2><ul>";

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) { // Check if /log exists and is a directory
      html += "<li>Немає доступних лог-файлів.</li>";
    } else {
      File file = root.openNextFile();
      while (file) {
        String path = file.name(); // /log/2025-06-27.csv
        if (!path.endsWith(".csv")) {
          file = root.openNextFile();
          continue;
        }

        String name = path;
        name.replace("/log/", "");
        name.replace(".csv", "");

        html += "<li>";
        html += "<a href=\"/log_viewer?date=" + name + "\">📊 " + name + "</a> ";
        html += "(<a href=\"" + path + "\">csv</a>)";
        html += "</li>";

        file = root.openNextFile();
      }
    }
    
    html += "</ul><p><a href=\"/\">На головну</a></p>";

    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });

  //===============================================================================
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });

  //===============================================================================


  //+++++++++++++++++++
  //////////
  server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request){
      if (request->hasParam("device_name", true))
          deviceName = request->getParam("device_name", true)->value();
      if (request->hasParam("ssid", true))
          wifiSSID = request->getParam("ssid", true)->value();

      if (request->hasParam("timezone_offset", true))
          tzOffset = request->getParam("timezone_offset", true)->value().toInt();

      useDST = request->hasParam("use_dst", true);

      // Формування tzString
      String sign = tzOffset >= 0 ? "-" : "+";
      int absOffset = abs(tzOffset);
      String tzString = "UTC" + sign + String(absOffset);
      if (useDST)
          tzString += "DST,M3.5.0/3,M10.5.0/4";

      setenv("TZ", tzString.c_str(), 1);
      tzset();

      switch1 = request->hasParam("switch1", true);
      switch2 = request->hasParam("switch2", true);
      switch3 = request->hasParam("switch3", true);
      switch4 = request->hasParam("switch4", true);
      switch5 = request->hasParam("switch5", true);
      switch6 = request->hasParam("switch6", true);
      switch7 = request->hasParam("switch7", true);

      saveConfig();

      request->send(200, "text/html",
        "<script>alert('Налаштування збережено!');location.href='/settings';</script>");
  });
  //+++++++++++++++++++++++

  // --- UPDATED /save_now handler to start the FreeRTOS task ---
  server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
      // Створення та запуск завдання FreeRTOS для збереження історії
      xTaskCreate(
        saveHistoryTask,       // Функція, що буде виконуватись як завдання
        "SaveHistory",         // Назва завдання (для дебагу)
        8192,                  // Розмір стека в байтах
        NULL,                  // Параметр, що передається в завдання (тут не потрібен)
        1,                     // Пріоритет завдання (0 - найнижчий. 1 - вже робочий)
        NULL                   // Дескриптор завдання (якщо потрібно його контролювати)
      );
      // Негайно відправляємо відповідь клієнту, не чекаючи завершення збереження
      request->send(200, "text/plain", "✅ Збереження архіву розпочато у фоновому режимі.");
  });
  //++++++++++++++++++++++++++

  // --- NEW handler for /log/*.csv files to serve them correctly ---
  server.on("/log/*.csv", HTTP_GET, [](AsyncWebServerRequest *request){
    String path = request->url(); // Отримуємо повний шлях, наприклад, "/log/2025-06-30.csv"
    if (SPIFFS.exists(path)) {
      // Важливо: встановлюємо Content-Type як text/csv та вказуємо charset=utf-8
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, "text/csv");
      response->addHeader("Content-Type", "text/csv; charset=utf-8"); // Додано charset
      request->send(response);
    } else {
      request->send(404, "text/plain", "Файл архіву не знайдено!");
    }
  });


  // --- UPDATED onNotFound handler ---
  server.onNotFound([](AsyncWebServerRequest *request){
    // Відправляємо HTML-сторінку 404 з коректним кодуванням UTF-8
    request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
  });

  // Запуск веб-сервера
  server.begin();
     currentDisplayMode = MODE_DATETIME; // Або якийсь інший режим за замовчуванням
    executeDisplayMode(currentDisplayMode); // Відобразити початковий режим
    displayModeStartTime = millis(); // Запустити таймер для цього режиму
   // displayTolyaVita();
     //delay(10000);
}

void loop() {
  unsigned long currentMillis = millis();
 
  // ==========================================================
  // Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ
  // Цей блок перевіряє, чи минула 5 хвилин (300000 мс) з останнього оновлення даних.
  // Він працює незалежно від того, що відображається на екрані.
  // ==========================================================
  static uint32_t lastUpdate = 0;
  if (currentMillis - lastUpdate > 300000) {
    lastUpdate = currentMillis;
    lastTemp = bme.readTemperature();
    lastHum = bme.readHumidity();
    lastPress = bme.readPressure();

    String timeStr = getCurrentTime();
    storeToHistory(lastTemp, lastHum, lastPress, timeStr);

    Serial.println("Sensor data updated and stored.");
  }

  // ==========================================================
  // Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
  // ==========================================================
  static String lastSavedDate;
  String today = getTodayDate();
  if (today != lastSavedDate) {
    performHistoryFileSave();
    clearOldLogs(30);
    clearHistoryData();
    lastSavedDate = today;
  }

  // ==========================================================
  // БЛОК КЕРУВАННЯ ДИСПЛЕЄМ (НОВА МАШИНА СТАНІВ ДЛЯ РОТАЦІЇ)
  // Всі режими виводяться ОДНОРАЗОВО при вході в режим.
  // ==========================================================

  // --- Фаза 1: Оновлення списку активних режимів для ротації ---
  // Оновлювати список активних режимів кожні 500 мс (залишаємо як є)
  static unsigned long lastModeListUpdate = 0;
  const unsigned long MODE_LIST_UPDATE_INTERVAL = 500; 

  if (currentMillis - lastModeListUpdate >= MODE_LIST_UPDATE_INTERVAL) {
    activeDisplayModes.clear(); 

    if (switch4) activeDisplayModes.push_back(MODE_TOLYA_VITA);
    if (switch6) activeDisplayModes.push_back(MODE_UKRAINIAN);
    if (switch7) activeDisplayModes.push_back(MODE_VIVAT);
    if (switch5) activeDisplayModes.push_back(MODE_SLAVA_UKRAINI);
    if (switch1) activeDisplayModes.push_back(MODE_DATETIME);
    if (switch3) activeDisplayModes.push_back(MODE_SHEVCHENKO);
    if (switch2) activeDisplayModes.push_back(MODE_BME_DATA);

    rotationActive = !activeDisplayModes.empty(); 
    lastModeListUpdate = currentMillis; 
  }

  // === ВАЖЛИВО: Зберегти поточний режим перед тим, як він може змінитися ===
  previousDisplayMode = currentDisplayMode; 

  // --- Фаза 2: Керування відображенням на основі списку ---
  switch (currentDisplayMode) {
    case MODE_NONE:
      if (rotationActive) {
        currentRotationIndex = 0; 
        currentDisplayMode = activeDisplayModes[currentRotationIndex];
        displayModeStartTime = currentMillis;
        
        // **Викликаємо executeDisplayMode ОДИН РАЗ при вході в новий режим**
        executeDisplayMode(currentDisplayMode); 
      } else {
        displayDefaultBackground();
      }
      break;

    // Спільна логіка для всіх сюжетів, що ротуються
    case MODE_TOLYA_VITA:
    case MODE_UKRAINIAN:
    case MODE_VIVAT:
    case MODE_SLAVA_UKRAINI:
    case MODE_DATETIME:
    case MODE_SHEVCHENKO:
    case MODE_BME_DATA:
      // В цьому блоці більше НЕМАЄ періодичних викликів displayDateTime(), displayBMEData(), displayTolyaVita() тощо.
      // Вміст режиму виводиться лише один раз при зміні currentDisplayMode.

      if (currentMillis - displayModeStartTime >= getDisplayDuration(currentDisplayMode)) {
        // Час поточного сюжету вичерпався. Переходимо до наступного або до MODE_NONE.
        
        if (rotationActive) { 
          // Визначити наступний режим у ротації
          currentRotationIndex = (currentRotationIndex + 1) % activeDisplayModes.size();
          DisplayMode nextMode = activeDisplayModes[currentRotationIndex];

          // **Логіка керування шрифтом My_ariali_26 (якщо потрібен):**
          // Якщо режим DATETIME використовує спеціальний шрифт, а інші ні.
          // Цей блок відповідає за те, щоб шрифт був завантажений ТІЛЬКИ для DATETIME
          // і вивантажений, коли виходимо з DATETIME.
          // Якщо ви хочете повністю ВИКЛЮЧИТИ встановлення цього шрифту,
          // видаліть цей if/else if блок повністю, і також приберіть loadFont/unloadFont
          // з функції executeDisplayMode, якщо вони там є.
          if (nextMode == MODE_DATETIME) {
              if (previousDisplayMode != MODE_DATETIME) {
                  tft.loadFont(My_ariali_26); // Завантажуємо шрифт тільки якщо переходимо в DATETIME
              }
          } else {
              if (previousDisplayMode == MODE_DATETIME) {
                  tft.unloadFont(); // Вивантажуємо шрифт тільки якщо виходимо з DATETIME
              }
          }
          // ********************************************************************

          currentDisplayMode = nextMode;
          displayModeStartTime = currentMillis;
          // **Викликаємо executeDisplayMode ОДИН РАЗ для нового режиму**
          executeDisplayMode(currentDisplayMode); 

        } else {
          // Якщо після завершення сюжету не залишилося активних перемикачів, повертаємося до MODE_NONE
          // **Вивантажуємо шрифт, якщо виходимо з DATETIME в MODE_NONE**
          if (previousDisplayMode == MODE_DATETIME) {
              tft.unloadFont();
          }
          currentDisplayMode = MODE_NONE;
        }
      }
      break;
  }
}

//End loop()
/*
void loop() {
  // Отримуємо поточний час у мілісекундах з моменту запуску плати.
  // Ця функція не зупиняє виконання коду, що дозволяє нам робити багато справ "одночасно".
  unsigned long currentMillis = millis();

  // ==========================================================
  // Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ
  // Цей блок перевіряє, чи минула 1 хвилина (60000 мс) з останнього оновлення даних.
  // Він працює незалежно від того, що відображається на екрані.
  // ==========================================================
  // У вашій функції loop()

static uint32_t lastUpdate = 0; // Статична змінна, зберігає час останнього оновлення
//unsigned long currentMillis = millis(); // Отримуємо поточний час

// Якщо пройшло більше 5 хвилин (300 000 мілісекунд) з останнього оновлення
if (currentMillis - lastUpdate > 300000) {
    lastUpdate = currentMillis; // Оновлюємо час останнього оновлення до поточного

    // Зчитуємо дані з датчика BME280 та присвоюємо їх ГЛОБАЛЬНИМ змінним
    lastTemp = bme.readTemperature();  // Температура в градусах Цельсія
    lastHum = bme.readHumidity();      // Вологість у відсотках
    lastPress = bme.readPressure();    // Тиск у Паскалях (Па). Цей формат використовується у displayBMEData() для конвертації в мм.рт.ст. та відображення в Па.

    // Змінна 'x'
    // Якщо 'x' - це глобальна змінна, яка ще десь використовується для тиску в Паскалях,
    // тоді ви можете залишити рядок:
    // x = lastPress;
    // Але якщо 'x' більше ніде не потрібна, ви можете її повністю видалити,
    // оскільки displayBMEData() використовує lastPress.

    String timeStr = getCurrentTime(); // Отримуємо поточний час у вигляді рядка (переконайтесь, що ця функція існує)
    storeToHistory(lastTemp, lastHum, lastPress, timeStr); // Зберігаємо дані в історію (переконайтесь, що ця функція існує)

    Serial.println("Sensor data updated and stored."); // Виводимо повідомлення в Serial
}
  /*
  static uint32_t lastUpdate = 0; // Статична змінна, зберігає час останнього оновлення
  if (currentMillis - lastUpdate > 300000) { // Якщо пройшло більше (60 секунд) 5хвилин
    lastUpdate = currentMillis; // Оновлюємо час останнього оновлення до поточного
    
    // Зчитуємо дані з датчика BME280
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    float rawPressure = bme.readPressure(); // Отримуємо сирий тиск
    lastPress = rawPressure / 100.0F;       // Конвертуємо в гектопаскалі (гПа)
    
    // Важливо: змінна 'x' для усереднення тиску.
    // У вашому старому коді було блокуюче усереднення (цикл з delay).
    // Щоб уникнути блокування, 'x' тепер просто присвоюється останнє виміряне значення.
    // Якщо вам потрібно справжнє усереднення, його слід реалізувати неблокуючим способом
    // або проводити рідше, тут або в окремій функціональності.
    x = rawPressure; // Присвоєння для використання у displayBMEData()

    String timeStr = getCurrentTime(); // Отримуємо поточний час у вигляді рядка
    storeToHistory(temperature, humidity, lastPress, timeStr); // Зберігаємо дані в масив історії
    
    Serial.println("Sensor data updated and stored."); // Виводимо повідомлення в Serial
  }
*/
/*
  // ==========================================================
  // Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
  // Цей блок перевіряє, чи змінилася доба. Якщо так, то архівує дані.
  // ==========================================================
  static String lastSavedDate; // Зберігає дату останнього архівування
  String today = getTodayDate(); // Отримуємо поточну дату
  if (today != lastSavedDate) { // Якщо дата змінилася (настала нова доба)
   //saveHistoryTask();   // Зберігаємо поточний масив історії в файл (архівуємо)
   performHistoryFileSave();   // <--- ТЕПЕР ВИКЛИКАЄМО НОВУ ФУНКЦІЮ! 
    clearOldLogs(30);      // Очищаємо старі лог-файли (старші за 30 днів)
    clearHistoryData();    // ОЧИЩУЄМО МАСИВ ПОТОЧНИХ ДАНИХ (1440 записів) для нової доби
    lastSavedDate = today; // Оновлюємо дату останнього архівування
  }

  // ==========================================================
  // БЛОК КЕРУВАННЯ ДИСПЛЕЄМ (НОВА 04.07.2025 МАШИНА СТАНІВ ДЛЯ РОТАЦІЇ)
  // ==========================================================

  static unsigned long lastModeListUpdate = 0;
  const unsigned long MODE_LIST_UPDATE_INTERVAL = 500;

  if (currentMillis - lastModeListUpdate >= MODE_LIST_UPDATE_INTERVAL) {
    activeDisplayModes.clear();
    if (switch4) activeDisplayModes.push_back(MODE_TOLYA_VITA);
    if (switch6) activeDisplayModes.push_back(MODE_UKRAINIAN);
    if (switch7) activeDisplayModes.push_back(MODE_VIVAT);
    if (switch5) activeDisplayModes.push_back(MODE_SLAVA_UKRAINI);
    if (switch1) activeDisplayModes.push_back(MODE_DATETIME);
    if (switch3) activeDisplayModes.push_back(MODE_SHEVCHENKO);
    if (switch2) activeDisplayModes.push_back(MODE_BME_DATA);
    rotationActive = !activeDisplayModes.empty();
    lastModeListUpdate = currentMillis;
  }

  previousDisplayMode = currentDisplayMode;

  switch (currentDisplayMode) {
    case MODE_NONE:
      if (rotationActive) {
        currentRotationIndex = 0;
        currentDisplayMode = activeDisplayModes[currentRotationIndex];
        displayModeStartTime = currentMillis;
        executeDisplayMode(currentDisplayMode); // Перший вивід при вході в режим
      } else {
        displayDefaultBackground();
      }
      break;

    case MODE_TOLYA_VITA:
    case MODE_UKRAINIAN:
    case MODE_VIVAT:
    case MODE_SLAVA_UKRAINI:
    case MODE_DATETIME:
    case MODE_SHEVCHENKO:
    case MODE_BME_DATA:
      // --- Логіка періодичного оновлення для КОЖНОГО режиму ---
      // Ви можете налаштувати інтервал оновлення для кожного режиму окремо
      // або використовувати спільний інтервал.
      // Наприклад, для зображень, 1-секундне оновлення - це, мабуть, надлишково,
      // але для демонстрації аналогії підійде.

      // ************ Додаємо сюди логіку оновлення ************
      static unsigned long lastModeContentUpdate = 0;
      const unsigned long IMAGE_UPDATE_INTERVAL = 5000; // Оновлювати зображення кожні 5 секунд
      const unsigned long DATETIME_UPDATE_INTERVAL = 1000; // Оновлювати час кожну секунду
      const unsigned long BME_UPDATE_INTERVAL = 5000; // Оновлювати BME кожні 5 секунд (або частіше)

      if (currentMillis - lastModeContentUpdate >= 
          (currentDisplayMode == MODE_DATETIME ? DATETIME_UPDATE_INTERVAL :
           currentDisplayMode == MODE_BME_DATA ? BME_UPDATE_INTERVAL :
           IMAGE_UPDATE_INTERVAL)) // Задаємо інтервал в залежності від режиму
      {
        // Для зображень ми просто повторно викликаємо функцію виведення.
        // Для часу та BME, ці функції вже вміють оновлювати дані на місці.
        // Перемальовуємо вміст поточного режиму
        if (currentDisplayMode == MODE_DATETIME) {
          // Шрифти вже завантажені на цьому етапі, не треба unload/load
          displayDateTime();
        } else if (currentDisplayMode == MODE_BME_DATA) {
          displayBMEData();
        } else if (currentDisplayMode == MODE_TOLYA_VITA) {
          // Якщо у вас є функція tft.unloadFont() або tft.loadFont(),
          // і ви підозрюєте, що вони викликаються тут не вчасно,
          // МОЖНА ДОДАТИ tft.unloadFont() ПЕРЕД ВИВОДОМ ЗОБРАЖЕННЯ для тестування.
          // Але це має бути вже оброблено в executeDisplayMode() при зміні режиму.
          displayTolyaVita(); // Перемальовуємо зображення
        } else if (currentDisplayMode == MODE_UKRAINIAN) {
          displayUkrainian(); // Перемальовуємо зображення
        }
        // ... додайте аналогічні блоки для інших режимів зображень, якщо потрібно
        else if (currentDisplayMode == MODE_VIVAT) {
          displayVivat();
        } else if (currentDisplayMode == MODE_SLAVA_UKRAINI) {
          displaySlavaUkraini();
        } else if (currentDisplayMode == MODE_SHEVCHENKO) {
          displayShevchenko();
        }
        
        lastModeContentUpdate = currentMillis; // Оновлюємо час останнього оновлення вмісту
      }
      // ************ Кінець блоку оновлення ************

      if (currentMillis - displayModeStartTime >= getDisplayDuration(currentDisplayMode)) {
        // Час поточного сюжету вичерпався. Переходимо до наступного або до MODE_NONE.
        if (rotationActive) {
          currentRotationIndex = (currentRotationIndex + 1) % activeDisplayModes.size();
          DisplayMode nextMode = activeDisplayModes[currentRotationIndex];

          // === Логіка вивантаження/завантаження шрифту при зміні режиму ===
          // Цей блок є критично важливим і має бути коректним.
          if (previousDisplayMode == MODE_DATETIME && nextMode != MODE_DATETIME) {
            tft.unloadFont();
          } else if (previousDisplayMode != MODE_DATETIME && nextMode == MODE_DATETIME) {
            tft.loadFont(My_ariali_26);
          }

          currentDisplayMode = nextMode;
          displayModeStartTime = currentMillis;
          executeDisplayMode(currentDisplayMode); // Перший вивід для нового режиму
        } else {
          if (previousDisplayMode == MODE_DATETIME) {
            tft.unloadFont();
          }
          currentDisplayMode = MODE_NONE;
        }
      }
      break;
  }
}

*/
//End loop()


/*
      // ************ Кінець блоку оновлення ************

      if (currentMillis - displayModeStartTime >= getDisplayDuration(currentDisplayMode)) {
        // Час поточного сюжету вичерпався. Переходимо до наступного або до MODE_NONE.
        
        if (rotationActive) { 
          // Визначити наступний режим у ротації
          currentRotationIndex = (currentRotationIndex + 1) % activeDisplayModes.size();
          DisplayMode nextMode = activeDisplayModes[currentRotationIndex];

          // === Логіка вивантаження/завантаження шрифту при зміні режиму ===
          // Якщо ми виходимо з режиму DATETIME і переходимо в інший режим
          if (previousDisplayMode == MODE_DATETIME && nextMode != MODE_DATETIME) {
              tft.unloadFont(); // Вивантажуємо шрифт My_ariali_26
          }
          // Якщо ми переходимо в режим DATETIME з іншого режиму
          else if (previousDisplayMode != MODE_DATETIME && nextMode == MODE_DATETIME) {
              tft.loadFont(My_ariali_26); // Завантажуємо шрифт My_ariali_26
          }
          // Примітка: Якщо переходимо з DATETIME на DATETIME (наприклад, у списку лише DATETIME),
          // шрифт не вивантажується/перезавантажується - це коректно.

          currentDisplayMode = nextMode;
          displayModeStartTime = currentMillis;
          executeDisplayMode(currentDisplayMode); // executeDisplayMode() тепер сама завантажує шрифт, якщо потрібно.

        } else {
          // Якщо після завершення сюжету не залишилося активних перемикачів, повертаємося до MODE_NONE
          // === Логіка вивантаження шрифту, якщо ми виходимо з DATETIME в MODE_NONE ===
          if (previousDisplayMode == MODE_DATETIME) {
              tft.unloadFont();
          }
          currentDisplayMode = MODE_NONE;
        }
      }
      break;
  }
}
*/
/*
  // ==========================================================
  // БЛОК КЕРУВАННЯ ДИСПЛЕЄМ (НОВА МАШИНА СТАНІВ ДЛЯ РОТАЦІЇ)
  // ==========================================================

  // --- Фаза 1: Оновлення списку активних режимів для ротації ---
  // Цей блок перевіряє стан switchX і формує список сюжетів, які мають ротуватися.
  // Він не повинен виконуватися занадто часто, щоб уникнути надмірних операцій з вектором.
  static unsigned long lastModeListUpdate = 0;
  const unsigned long MODE_LIST_UPDATE_INTERVAL = 500; // Оновлювати список активних режимів кожні 500 мс

  if (currentMillis - lastModeListUpdate >= MODE_LIST_UPDATE_INTERVAL) {
      activeDisplayModes.clear(); // Очищаємо список перед формуванням нового

      // Додаємо режими до списку, якщо відповідний switch увімкнений
      if (switch4) activeDisplayModes.push_back(MODE_TOLYA_VITA);
      if (switch6) activeDisplayModes.push_back(MODE_UKRAINIAN);
      if (switch7) activeDisplayModes.push_back(MODE_VIVAT);
      if (switch5) activeDisplayModes.push_back(MODE_SLAVA_UKRAINI);
      if (switch1) activeDisplayModes.push_back(MODE_DATETIME);
      if (switch3) activeDisplayModes.push_back(MODE_SHEVCHENKO);
      if (switch2) activeDisplayModes.push_back(MODE_BME_DATA);

      rotationActive = !activeDisplayModes.empty(); // Встановлюємо прапор, чи є що ротувати
      lastModeListUpdate = currentMillis; // Оновлюємо час останнього оновлення списку
  }

    // === ВАЖЛИВО: Зберегти поточний режим перед тим, як він може змінитися ===
  previousDisplayMode = currentDisplayMode; 

  // --- Фаза 2: Керування відображенням на основі списку ---
  switch (currentDisplayMode) {
    case MODE_NONE:
      if (rotationActive) {
        currentRotationIndex = 0; 
        currentDisplayMode = activeDisplayModes[currentRotationIndex];
        displayModeStartTime = currentMillis;
        // executeDisplayMode() тепер сама завантажує шрифт для MODE_DATETIME
        executeDisplayMode(currentDisplayMode); 
      } else {
        displayDefaultBackground();
      }
      break;

    // Спільна логіка для всіх сюжетів, що ротуються
    case MODE_TOLYA_VITA:
    case MODE_UKRAINIAN:
    case MODE_VIVAT:
    case MODE_SLAVA_UKRAINI:
    case MODE_DATETIME:
    case MODE_SHEVCHENKO:
    case MODE_BME_DATA:
    /*
      // === ВАЖЛИВА ЗМІНА ТУТ: ПОВЕРТАЄМО щосекундне оновлення для MODE_DATETIME ===
      if (currentDisplayMode == MODE_DATETIME) {
        static unsigned long lastTimeUpdate = 0; // Ця змінна відстежує час останнього оновлення
        if (currentMillis - lastTimeUpdate >= 1000) { // Оновлюємо кожну секунду
          displayDateTime(); // Цей виклик ПОВЕРТАЄТЬСЯ
          lastTimeUpdate = currentMillis;
        }
      }
      */
      // =========================================================================
          // === ВАЖЛИВА ЗМІНА ТУТ: Видаляємо весь блок щосекундного оновлення для MODE_DATETIME ===
      // if (currentDisplayMode == MODE_DATETIME) {
      //   static unsigned long lastTimeUpdate = 0; 
      //   if (currentMillis - lastTimeUpdate >= 1000) {
      //     displayDateTime(); // ЦЕЙ ВИКЛИК БУДЕ ВИДАЛЕНО
      //     lastTimeUpdate = currentMillis;
      //   }
      // }
      // =========================================================================================
      /*
      // Оновлення часу для MODE_DATETIME, якщо це поточний режим
      if (currentDisplayMode == MODE_DATETIME) {
        static unsigned long lastTimeUpdate = 0;
        if (currentMillis - lastTimeUpdate >= 1000) {
          displayDateTime(); // Оновлюємо лише дані, без завантаження/вивантаження шрифту
          lastTimeUpdate = currentMillis;
        }
      }
      */

/*
  // --- Фаза 2: Керування відображенням на основі списку ---
  switch (currentDisplayMode) {
    case MODE_NONE:
      if (rotationActive) {
        // Якщо є активні режими, починаємо ротацію з першого.
        // Запускаємо перший сюжет зі списку активних режимів.
        currentRotationIndex = 0; // Починаємо з першого елемента списку
        currentDisplayMode = activeDisplayModes[currentRotationIndex];
        displayModeStartTime = currentMillis;
        executeDisplayMode(currentDisplayMode); // Викликаємо допоміжну функцію для відображення
      } else {
        // Якщо немає активних режимів (всі switchX вимкнені), показуємо стандартний фон.
        displayDefaultBackground();
      }
      break;

    // Спільна логіка для ВСІХ сюжетів, що ротуються.
    // Тепер всі DisplayMode (окрім MODE_NONE) обробляються однаково.
    case MODE_TOLYA_VITA:
    case MODE_UKRAINIAN:
    case MODE_VIVAT:
    case MODE_SLAVA_UKRAINI:
    case MODE_DATETIME:
    case MODE_SHEVCHENKO:
    case MODE_BME_DATA:
      // Оновлення часу для MODE_DATETIME, якщо це поточний режим
      if (currentDisplayMode == MODE_DATETIME) {
        static unsigned long lastTimeUpdate = 0;
        if (currentMillis - lastTimeUpdate >= 1000) {
          displayDateTime(); // Оновлюємо час щосекунди
          lastTimeUpdate = currentMillis;
        }
      }

      // Перевіряємо, чи минув час відображення поточного сюжету
      unsigned long duration = getDisplayDuration(currentDisplayMode);

      if (currentMillis - displayModeStartTime >= duration) {
        // Час поточного сюжету вичерпався. Переходимо до наступного або до MODE_NONE.

        // Спочатку перевіримо, чи поточний режим все ще повинен бути активним
        // (тобто, чи відповідний switchX все ще увімкнений).
        // Якщо ні, ми його пропускаємо і шукаємо наступний активний.
        bool currentModeStillActive = false;
        if (!activeDisplayModes.empty() && currentRotationIndex < activeDisplayModes.size()) {
            // Перевіряємо, чи поточний режим в activeDisplayModes.
            // Найпростіше - перевірити, чи activeDisplayModes[currentRotationIndex] == currentDisplayMode
            // Але оскільки список оновлюється періодично, може бути, що currentRotationIndex
            // вказує на режим, який вже був вимкнений.
            // Тому краще шукати наступний дійсний режим.
            currentRotationIndex = (currentRotationIndex + 1) % activeDisplayModes.size();
            
            // Якщо список порожній або поточний індекс вийшов за межі, то ротації немає.
            if (!activeDisplayModes.empty()) {
                currentDisplayMode = activeDisplayModes[currentRotationIndex];
                displayModeStartTime = currentMillis;
                executeDisplayMode(currentDisplayMode); // Викликаємо наступний сюжет
            } else {
                // Якщо після оновлення списку активних режимів не залишилося
                currentDisplayMode = MODE_NONE;
            }

        } else { // Якщо activeDisplayModes раптом порожній або індекс не дійсний
             currentDisplayMode = MODE_NONE;
        }
      }
      break;
  }
}
*/
/*

  // ==========================================================
  // МАШИНА СТАНІВ ДЛЯ КЕРУВАННЯ ВІДОБРАЖЕННЯМ НА TFT-ДИСПЛЕЇ
  // Цей блок вирішує, що показувати на екрані, і як довго.
  // Вона не блокує виконання решти коду.
  // ==========================================================
  switch (currentDisplayMode) {
    case MODE_NONE:
      // Це "стан спокою". Ми тут, коли жоден специфічний режим відображення не активний
      // або щойно завершився попередній режим.
      
      displayDefaultBackground(); // Показуємо наш стандартний фон (синьо-жовтий)

      // Тепер перевіряємо, який перемикач (switch) увімкнений і визначаємо,
      // який режим відображення має розпочатися наступним.
      // Пріоритет тут задається порядком перевірок (від switch4 до switch2).
      if (switch4) {
        currentDisplayMode = MODE_TOLYA_VITA;   // Встановлюємо новий режим
        displayModeStartTime = currentMillis;   // Записуємо поточний час як час старту режиму
        displayTolyaVita();                     // Викликаємо функцію, яка малює цей екран
      } else if (switch6) {
        currentDisplayMode = MODE_UKRAINIAN;
        displayModeStartTime = currentMillis;
        displayUkrainian();
      } else if (switch7) {
        currentDisplayMode = MODE_VIVAT;
        displayModeStartTime = currentMillis;
        displayVivat();
      } else if (switch5) {
        currentDisplayMode = MODE_SLAVA_UKRAINI;
        displayModeStartTime = currentMillis;
        displaySlavaUkraini();
      } else if (switch1) {
        currentDisplayMode = MODE_DATETIME;
        displayModeStartTime = currentMillis;
        displayDateTime();
      } else if (switch3) {
        currentDisplayMode = MODE_SHEVCHENKO;
        displayModeStartTime = currentMillis;
        displayShevchenko();
      } else if (switch2) {
        currentDisplayMode = MODE_BME_DATA;
        displayModeStartTime = currentMillis;
        displayBMEData();
      }
      // Якщо жоден з перемикачів не активний, ми залишаємось у MODE_NONE
      // і будемо продовжувати показувати фон, поки якийсь перемикач не увімкнеться.
      break;

    case MODE_TOLYA_VITA:
      // Ми знаходимося в режимі відображення зображення Толі і Віти.
      // Перевіряємо, чи минув час, відведений для цього режиму (DURATION_TOLYA_VITA).
      if (currentMillis - displayModeStartTime >= DURATION_TOLYA_VITA) {
        currentDisplayMode = MODE_NONE; // Якщо час минув, переходимо назад у "стан спокою"
      }
      // Якщо час ще не минув, ми нічого не робимо в цьому `case`.
      // Це дозволяє `loop()` продовжувати виконуватися дуже швидко,
      // обробляючи інші завдання (веб-сервер, датчики).
      break;

    case MODE_UKRAINIAN:
      if (currentMillis - displayModeStartTime >= DURATION_UKRAINIAN) {
        currentDisplayMode = MODE_NONE;
      }
      break;

    case MODE_VIVAT:
      if (currentMillis - displayModeStartTime >= DURATION_VIVAT) {
        currentDisplayMode = MODE_NONE;
      }
      break;

    case MODE_SLAVA_UKRAINI:
      if (currentMillis - displayModeStartTime >= DURATION_SLAVA_UKRAINI) {
        currentDisplayMode = MODE_NONE;
      }
      break;

    case MODE_DATETIME:
      // У цьому режимі ми відображаємо дату і час.
      // Час змінюється кожну секунду, тому ми хочемо оновлювати екран частіше.
      if (currentMillis - displayModeStartTime >= DURATION_DATETIME) {
        currentDisplayMode = MODE_NONE; // Час відображення режиму минув
      } else {
        // Якщо режим ще активний, перевіряємо, чи минула 1 секунда для оновлення секунд на дисплеї.
        static unsigned long lastTimeUpdate = 0; // Зберігає час останнього оновлення часу на дисплеї
        if (currentMillis - lastTimeUpdate >= 1000) { // Якщо пройшла 1 секунда
          displayDateTime(); // Перемалюємо час, щоб оновити секунди (та хвилини/години)
          lastTimeUpdate = currentMillis; // Оновлюємо час останнього оновлення дисплея
        }
      }
      break;

    case MODE_SHEVCHENKO:
      if (currentMillis - displayModeStartTime >= DURATION_SHEVCHENKO) {
        currentDisplayMode = MODE_NONE;
      }
      break;

    case MODE_BME_DATA:
      // У цьому режимі ми відображаємо дані з BME280.
      // Ми також можемо оновлювати ці дані періодично, якщо вони змінюються.
      if (currentMillis - displayModeStartTime >= DURATION_BME_DATA) {
        currentDisplayMode = MODE_NONE; // Час відображення режиму минув
      } else {
        // Якщо режим ще активний, оновлюємо дані BME280 на дисплеї кожні 5 секунд.
        static unsigned long lastBMEUpdate = 0; // Зберігає час останнього оновлення даних BME на дисплеї
        if (currentMillis - lastBMEUpdate >= 5000) { // Якщо пройшло 5 секунд
          displayBMEData(); // Перемалюємо дані BME
          lastBMEUpdate = currentMillis; // Оновлюємо час останнього оновлення дисплея
        }
      }
      break;
  }
  
  // ==========================================================
  // КІНЕЦЬ БЛОКУ МАШИНИ СТАНІВ
  // ==========================================================

  // Важливо: Тут немає `delay()`. Функція `loop()` виконується безперервно,
  // що забезпечує максимальну відгуковість вашого ESP32.
}
*/

// ==== Нова функція: Основний функціонал збереження історії у файл ====
// Ця функція містить код, який безпосередньо записує дані в SPIFFS.
// Вона не приймає аргументів і не має нескінченних циклів чи затримок,
// тому її можна безпечно викликати з void loop().
void performHistoryFileSave() {
  File file = SPIFFS.open("/history.csv", FILE_APPEND); // Відкриваємо файл для дозапису
  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл історії для дозапису!");
    return; // Якщо не вдалося відкрити, виходимо
  }

  // Припускаємо, що 'history' та 'MAX_MEASUREMENTS' є глобально доступними
  // Цей цикл записує всі дані з масиву 'history' у файл
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    if (history[i].timeStr != "") { // Записуємо лише якщо є дійсні дані
      file.printf("%s,%.2f,%.2f,%.2f\n", history[i].timeStr.c_str(),
                  history[i].temperature, history[i].humidity, history[i].pressure);
    }
  }
  file.close(); // Закриваємо файл
  Serial.println("✅ Дані історії збережено у файл.");
}
// У вашому файлі subroutines.cpp або main.cpp
// src/subroutines.cpp

// ... (існуючий код)

void executeDisplayMode(DisplayMode mode) {
    // Очистити екран перед відображенням нового режиму
    // (Цей рядок може бути тут, або в кожній функції режиму, якщо вони малюють поверх попереднього)
    tft.fillScreen(TFT_BLACK); 

    switch (mode) {
        case MODE_NONE:
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("Режим неактивний", screenW / 2, screenH / 2);
            break;
        case MODE_TOLYA_VITA:
            // Ваша логіка для Tolya_Vita
            // ...
            break;
        case MODE_UKRAINIAN:
            // Ваша логіка для Ukrainian
            // ...
            break;
        case MODE_VIVAT:
            vivat(); // Приклад виклику функції
            break;
        case MODE_SLAVA_UKRAINI:
            slava_ukraini(); // Приклад виклику функції
            break;
        case MODE_DATETIME:
            displayDateTime(); // Викликаємо функцію для відображення часу та дати
            break;
        case MODE_SHEVCHENKO:
            shevchenko(); // Приклад виклику функції
            break;
        case MODE_BME_DATA:
            displayBMEData(); // <--- ТУТ ВАЖЛИВИЙ ВИКЛИК!
            break;
        default:
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("Невідомий режим", screenW / 2, screenH / 2);
            break;
    }
}

// ... (кінець subroutines.cpp)
/*
void executeDisplayMode(DisplayMode mode) {
  tft.fillScreen(TFT_BLACK); // Очищаємо весь екран при зміні режиму

  // Тут не потрібно loadFont/unloadFont, оскільки шрифт завантажений у setup()

  switch (mode) {
    case MODE_TOLYA_VITA:     displayTolyaVita(); break;
    case MODE_UKRAINIAN:      displayUkrainian(); break;
    case MODE_VIVAT:          displayVivat(); break;
    case MODE_SLAVA_UKRAINI:  displaySlavaUkraini(); break;
    case MODE_DATETIME:       displayDateTime(); break; // Викликаємо displayDateTime для первинного малювання
    case MODE_SHEVCHENKO:     displayShevchenko(); break;
    case MODE_BME_DATA:       displayBMEData(); break;
    case MODE_NONE:           displayDefaultBackground(); break;
  }
}
*/
/*
// Реалізація executeDisplayMode() (наприклад, у subroutines.cpp або main.cpp)
void executeDisplayMode(DisplayMode mode) {
  tft.fillScreen(TFT_BLACK); // Це коректно очищає екран один раз при вході в новий режим

  // Якщо новий режим - це режим дати/часу, завантажуємо потрібний шрифт
  if (mode == MODE_DATETIME) {
      tft.loadFont(My_ariali_26); // Завантажуємо шрифт My_ariali_26
  }
  // Додайте тут схожі блоки, якщо інші режими використовують інші спеціальні шрифти
  // else if (mode == MODE_BME_DATA) { tft.loadFont(AnotherFont); }


  switch (mode) {
    case MODE_TOLYA_VITA:     displayTolyaVita(); break;
    case MODE_UKRAINIAN:      displayUkrainian(); break;
    case MODE_VIVAT:          displayVivat(); break;
    case MODE_SLAVA_UKRAINI:  displaySlavaUkraini(); break;
    case MODE_DATETIME:       displayDateTime(); break; // displayDateTime() тепер НЕ завантажує/вивантажує шрифт
    case MODE_SHEVCHENKO:     displayShevchenko(); break;
    case MODE_BME_DATA:       displayBMEData(); break;
    case MODE_NONE:           displayDefaultBackground(); break;
  }
}
*/
/*
// ==== Допоміжна функція: Виконання відображення для заданого режиму ====
// Ця функція приймає DisplayMode і викликає відповідну функцію малювання.
void executeDisplayMode(DisplayMode mode) {
  // Завжди очищаємо екран перед малюванням нового сюжету, щоб уникнути артефактів
  tft.fillScreen(TFT_BLACK);

  switch (mode) {
    case MODE_TOLYA_VITA:     displayTolyaVita(); break;
    case MODE_UKRAINIAN:      displayUkrainian(); break;
    case MODE_VIVAT:          displayVivat(); break;
    case MODE_SLAVA_UKRAINI:  displaySlavaUkraini(); break;
    case MODE_DATETIME:       displayDateTime(); break;
    case MODE_SHEVCHENKO:     displayShevchenko(); break;
    case MODE_BME_DATA:       displayBMEData(); break;
    case MODE_NONE:           displayDefaultBackground(); break; // На випадок, якщо викликано для MODE_NONE
  }
}
*/
// ==== Допоміжна функція: Отримання тривалості для заданого режиму ====
// Повертає тривалість відображення для кожного DisplayMode.
// Функція, яка повертає тривалість відображення для кожного режиму
unsigned long getDisplayDuration(DisplayMode mode) {
    switch (mode) {
        case MODE_TOLYA_VITA:
            return 10000; // 5 секунд для зображення "Толя і Віта"
        case MODE_UKRAINIAN:
            return 5000; // 5 секунд для українського зображення
        case MODE_VIVAT:
            return 5000; // 5 секунд для зображення "Віват"
        case MODE_SLAVA_UKRAINI:
            return 5000; // 5 секунд для зображення "Слава Україні"
        case MODE_DATETIME:
            return 10000; // 10 секунд для відображення часу та дати
        case MODE_SHEVCHENKO:
            return 8000; // 8 секунд для зображення Шевченка
        case MODE_BME_DATA:
            return 15000; // 15 секунд для даних BME280
        case MODE_NONE:
        default:
            return 3000; // Стандартна тривалість або для невизначених режимів (наприклад, фон)
    }
}
/*
unsigned long getDisplayDuration(DisplayMode mode) {
  switch (mode) {
    case MODE_TOLYA_VITA:     return DURATION_TOLYA_VITA;
    case MODE_UKRAINIAN:      return DURATION_UKRAINIAN;
    case MODE_VIVAT:          return DURATION_VIVAT;
    case MODE_SLAVA_UKRAINI:  return DURATION_SLAVA_UKRAINI;
    case MODE_DATETIME:       return DURATION_DATETIME;
    case MODE_SHEVCHENKO:     return DURATION_SHEVCHENKO;
    case MODE_BME_DATA:       return DURATION_BME_DATA;
    default:                  return 5000; // Дефолтна тривалість (наприклад, 5 секунд) на випадок неочікуваного режиму
  }
}
*/


/*
void loop() {
    static uint32_t lastBMEUpdate = 0;
    static uint32_t lastDisplayUpdate = 0;
    static String lastSavedDate = ""; // Ініціалізація для відстеження останньої дати збереження
    const uint32_t BME_UPDATE_INTERVAL = 300000; //60000; // 60 секунд
    const uint32_t DISPLAY_UPDATE_INTERVAL = 5000; // 5 секунд для оновлення дисплея, якщо не в AP-режимі
  
    // Оновлення BME даних та історії
    if (millis() - lastBMEUpdate > BME_UPDATE_INTERVAL) {
        lastBMEUpdate = millis();
        float temp = bme.readTemperature();
        float hum = bme.readHumidity();
        float pres = bme.readPressure() / 100.0F;

        String timeStr = getCurrentTime(); // Використовуємо оновлену getCurrentTime()
        storeToHistory(temp, hum, pres, timeStr);







        lastTemp = temp;
        lastHum = hum;
        lastPress = pres;

        Serial.printf("Free Heap: %d / Min Free Heap: %d\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());

        String today = getTodayDate();
        // Якщо дата змінилася (новий день)
        if (today != lastSavedDate) {
          Serial.println("DEBUG: Daily save triggered.");
          xTaskCreate(
            saveHistoryTask,       // Функція, що буде виконуватись як завдання
            "DailySaveHistory",    // Назва завдання
            8192,                  // Розмір стека
            NULL,                  // Параметр
            1,                     // Пріоритет
            NULL                   // Дескриптор
          );
          clearOldLogs(30); // Видаляємо лог-файли старші за 30 днів
          lastSavedDate = today; // Оновлюємо дату останнього збереження
          // НЕ ОБНУЛЯЄМО historyIndex = 0; якщо хочемо, щоб графік продовжувався через північ
        }
    }

    // Оновлення TFT дисплея (умовне, щоб не заважати AP-режиму)
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = millis();

        tft.setRotation(1);
        tft.setTextWrap(false);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);

        if (WiFi.getMode() == WIFI_AP) {
            // У режимі AP показуємо лише основну інформацію для налагодження
            String apIP = WiFi.softAPIP().toString();
            tft.drawString("AP Mode", 10, 20);
            tft.drawString("SSID: ESP32_Config", 10, 40);
            tft.drawString("IP: " + apIP, 10, 60);
            tft.drawString("Pass: 12345678", 10, 80);
            tft.drawString("Free Heap: " + String(ESP.getFreeHeap()), 10, 120);
        } else {
            // У режимі STA або якщо немає активної AP, показуємо повну інформацію
            if (switch4) {
                tft.pushImage(0, 0, 320, 240, IMG_Tolya_Vita320_240);
                delay(5000); // Короткий delay, щоб не блокувати
            }
            // (можливо, тут потрібно очистити екран після зображення, якщо воно не займає весь екран)

            tft.fillScreen(TFT_BLACK); // Заповнюємо екран чорним перед новим малюнком

            if (switch6) {
                tft.pushImage(0, 0, 320, 240, IMG_ukrainian320_240);
                delay(5000);
            }
            tft.fillScreen(TFT_BLACK);

            tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
            tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);

            if (switch7) {
                tft.loadFont(My_ariali_26);
                vivat();
                tft.unloadFont();
                delay(5000);
            }
            if (switch5) {
                tft.loadFont(My_ariali_34);
                slava_ukraini();
                tft.unloadFont();
            }
            tft.fillScreen(TFT_BLACK);

            if (switch1) {
                tft.loadFont(My_ariali_26);
                DateTime now = rtc.now();
                day = now.day(); month = now.month(); year = now.year();
                hour = now.hour(); minute = now.minute(); second = now.second();
                week = now.dayOfTheWeek();

                tft.setCursor(80, 30);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                if(hour<10) { tft.print(0); tft.print(hour);} else { tft.print(hour); }
                tft.drawString(":", 110, 30);
                tft.setCursor(120, 30);
                if(minute<10) { tft.print(0); tft.print(minute); } else { tft.print(minute); }
                tft.setCursor(40, 80);
                if(day<10) { tft.print(0); tft.print(day); } else { tft.print(day); }
                tft.drawString(":",75, 80);
                tft.setCursor(85, 80);
                if(month<10) { tft.print(0); tft.print(month); } else { tft.print(month); }
                tft.drawString(":",120, 80);
                tft.setCursor(135, 80);
                tft.print(year);
                tft.unloadFont();
                week_day_out(week);
                delay(5000);
            }

            if (switch3) {
                tft.loadFont(My_ariali_20);
                shevchenko();
                tft.unloadFont();
                delay(5000);
            }

            if (switch2) {
                i=10; x=0;
                while(i) { x=x+bme.readPressure(); i--; }
                x=x/10;

                tft.fillScreen(TFT_BLACK);
                tft.loadFont(My_ariali_24);
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.drawString("Атмосферний тиск:", 30, 20);
                tft.setCursor(50, 50);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.print(x/133.322);
                tft.drawString(" mmHg", 140, 50);
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.setCursor(50, 80);
                tft.print(x);
                tft.drawString(" Pa", 140, 80);

                temperature = bme.readTemperature();
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.drawString("Температура: ", 20, 110);
                tft.setCursor(190, 110);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.print(temperature);
                tft.drawString(" C", 250, 110);

                humidity = bme.readHumidity();
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.drawString("Вологість:", 20, 170);
                tft.setCursor(155, 170);
                tft.print(humidity);
                tft.drawString(" %", 210, 170);
                delay(5000);
            }
        }
    }
    // Дуже важлива функція для FreeRTOS, дозволяє іншим задачам працювати.
    delay(10); // Коротка затримка, щоб дозволити іншим завданням (напр., веб-серверу) працювати
}
*/