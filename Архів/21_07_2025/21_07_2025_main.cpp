// Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ рядок 1052
// Очистити старі файли логів (3 дні)  clearOldLogs(3); рядок 575  clearOldLogs(2)
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
#include <time.h> 
#include <set>

AsyncWebServer server(80);
Preferences preferences;

String deviceName = "ESP32_Weather";
String wifiSSID = "Unknown";
String tzString = "EET-2EEST,M3.5.0/3,M10.5.0/4";

bool useDST = true;       // Літній час
int tzOffset = 2;  // за замовчуванням для Києва (UTC+2)
// Ініціалізація глобальних змінних для ротації дисплея
//std::vector<DisplayMode> activeDisplayModes; Перенесено до globals.h
//int currentRotationIndex = 0; Перенесено до globals.h
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
int flag_test =1;
int lastSavedIndex = 0;  // глобальна змінна, ініціалізована один раз
const int screenW = 320; // Встановіть тут фактичну ширину вашого екрану
const int screenH = 240; // Встановіть тут фактичну висоту вашого екрану
const int lineHeight = 28; // Встановіть бажану висоту лінії для тексту, наприклад, 28-30px для шрифту 26pt. Можете використовувати tft.fontHeight() + відступ.
String lastArchivedTimestamp = "";
// ... решта ваших глобальних змінних

bool switch1 = false, switch2 = false, switch3 = false, switch4 = false;
bool switch5 = false, switch6 = false, switch7 = false;
float lastTemp = 0, lastHum = 0, lastPress = 0;
float temperature;
float pressure;
float humidity;
int8_t   hour, minute, second;
long x;//усереднення тиску
// === 🔧 ЗМІННІ ДЛЯ ІНДЕКСУВАННЯ АРХІВАЦІЇ ===
//int historyIndex = 0;
int lastArchivedIndex = 0;

unsigned char flag_s=1;
unsigned char flag_w=1;
unsigned char  ind;
bool testArchiveFlag = true; //Флаг тестового архівування
#define MAX_POINTS 1440
#define MAX_MEASUREMENTS 1440

struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

// Масив для збереження історії
const int maxHistorySize = 1440;
BMEData history[maxHistorySize];



//BMEData history[MAX_MEASUREMENTS];
int historyIndex = 0;
//+++++++++++++++=============

//DisplayMode currentDisplayMode = MODE_NONE;
unsigned long displayModeStartTime = 0; // Час початку поточного режиму відображення

// Оголошення нових допоміжних функцій для відображення
void displayTolyaVita();
void displayUkrainian();
void displayVivat();
void displaySlavaUkraini();
void displayDateTime();
void displayShevchenko();
void displayBMEData();
void displayDefaultBackground();
void saveHistoryTask(void *parameter);
void deleteLogFile(const char* filename);
String getCurrentDate();
//void saveHistoryTask();
void performHistoryFileSave();// Оголошення нової функції для збереження історії

void resetHistoryForNewDay();
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr);

//void clearOldLogs(int days); // Оголошення для функції очищення старих логів
void clearHistoryData();     // Оголошення для функції очищення масиву поточних даних
// Допоміжні функції для відображення на TFT

void displayTolyaVita() {
//Serial.println("displayTolyaVita() викликано. Спроба виведення зображення TolyaVita."); // Додано для налагодження
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
//+++++++++++++++++++++++++++++++++++
//повертає дату у форматі YYYY-MM-DD, як назва файлу архіву.
String getCurrentDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}

//+++++++++++++++++++++++++++++++++++
//++++++++++++++==============
// --- Декларації наперед (збережені з оригіналу) ---
String getTodayDate();
void clearOldLogs(int maxDays);



//////////////
String buildTZString(int offsetHours, bool useDST) {
  String sign = offsetHours >= 0 ? "-" : "+";
  int absOffset = abs(offsetHours);
  String tz = "GMT" + sign + String(absOffset);
  if (useDST) tz += "DST,M3.5.0/3,M10.5.0/4";
  return tz;
}

///////////////Побудова графіків
//Додано очищення найстарішого запису, якщо масив заповнений.
// Оптимізовано - забрано не потрібні перевірки на дублікати
void storeToHistory(float temp, float hum, float pres, const String& timeStr) {
  if (historyIndex >= MAX_MEASUREMENTS) {
    // Зсув усіх записів вліво (видаляємо найстаріший)
    for (int i = 1; i < MAX_MEASUREMENTS; ++i) {
      history[i - 1] = history[i];
    }
    historyIndex = MAX_MEASUREMENTS - 1;
  }

  history[historyIndex].temperature = temp;
  history[historyIndex].humidity = hum;
  history[historyIndex].pressure = pres;
  history[historyIndex].timeStr = timeStr;
  historyIndex++;

  Serial.printf("💾 Збережено запис #%d: %s | %.2f°C %.2f%% %.2f hPa\n",
                historyIndex, timeStr.c_str(), temp, hum, pres);
}


/*
//Перевірка на дублікати за timeStr — уникнення повторів.
//17.07.2025
void storeToHistory(float temp, float hum, float pres, const String& timeStr) {
  // Перевірка на дублікати
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    if (history[i].timeStr == timeStr) {
      Serial.println("⚠️ Запис з таким часом вже існує, пропускаємо.");
      return;
    }
  }

  if (historyIndex >= MAX_MEASUREMENTS) {
    // Зсув усіх записів вліво (видаляємо найстаріший)
    for (int i = 1; i < MAX_MEASUREMENTS; ++i) {
      history[i - 1] = history[i];
    }
    historyIndex = MAX_MEASUREMENTS - 1;
  }

  history[historyIndex].temperature = temp;
  history[historyIndex].humidity = hum;
  history[historyIndex].pressure = pres;
  history[historyIndex].timeStr = timeStr;
  historyIndex++;

  Serial.printf("💾 Збережено запис #%d: %s | %.2f°C %.2f%% %.2f hPa\n",
                historyIndex, timeStr.c_str(), temp, hum, pres);
}
*/

/*
void storeToHistory(float temp, float hum, float pres, String timeStr) {
  history[historyIndex] = { temp, hum, pres, timeStr };
  historyIndex = (historyIndex + 1) % MAX_POINTS;
}
*/
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
//++++++++++++++++++++++++++++++++++
//Функція для "короткого" часу HH:MM:SS для запуску задачі архівування
String getShortTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    DateTime now = rtc.now();
    char buf[6];
    sprintf(buf, "%02d:%02d", now.hour(), now.minute());
    return String(buf);
  }
  char buf[6];
  strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
  return String(buf);
}

//+++++++++++++++++++++++++++++++++
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
//======++++++++++++++++++++++++
/* Функція не перевірялася, так як попередня функція нібито працює
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  // 🧪 Діагностика масиву history[]
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s", i, d.timeStr.c_str());
    }
    written++;
  }

  file.flush();  // 💾 Гарантоване скидання буфера
  file.close();  // 🔐 Закриття файлу
  delay(50);     // 💤 Дати SPIFFS завершити запис

  Serial.printf("✅ Архів завершено: %s (%d записів)", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData(); // ✅ автоматично очищаємо
    clearOldLogs(3);	// ✅ і видаляємо старі архіви
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}
*/


//++++++++++++++++++++++++++++++++++++++
//Усунення зайвих пробілів у масиві history[]
//Варіант від 19.07.2025 (див. нижче) Зміни в частині перевірки наявності Директорії /log 
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.printf("➡️  Файл: %s\n", filename.c_str());

// ✅ Перевірка наявності директорії /log
  File testDir = SPIFFS.open("/log");
  if (!testDir || !testDir.isDirectory()) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  } else {
    Serial.println("📁 Директорія /log існує.");
  }
  testDir.close();
//Перевірка завершена

  if (clearAfterSave) {
    Serial.println("🔁 Тип: Автоматична архівація (з очищенням history[])");
  } else {
    Serial.println("🛠️ Тип: Ручна архівація (без очищення history[])");
  }
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);

  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);  // UTF-8 BOM
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // оновлюємо лічильник
  lastSavedIndex = historyIndex;
  Serial.printf("📌 Оновлено lastSavedIndex → %d\n", lastSavedIndex);

  if (clearAfterSave) {
  clearHistoryData(); // очищення масиву поточних даних
  Serial.println("🧹 Масив history[] очищено після архівації");

  Serial.println("♻️ Виклик clearOldLogs(3)...");  // Додано
  clearOldLogs(3);  // очищення старих архівів
  Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
}
  vTaskDelete(NULL);
}

/*

//Працюючий варіант. Є підозра, що формує масив даних з зайвими пробілами після коми
//Варіант від 19.07.2025 (див. нижче) Зміни в частині перевірки наявності Директорії /log 
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.printf("➡️  Файл: %s\n", filename.c_str());

// ✅ Перевірка наявності директорії /log
  File testDir = SPIFFS.open("/log");
  if (!testDir || !testDir.isDirectory()) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  } else {
    Serial.println("📁 Директорія /log існує.");
  }
  testDir.close();
//Перевірка завершена

  if (clearAfterSave) {
    Serial.println("🔁 Тип: Автоматична архівація (з очищенням history[])");
  } else {
    Serial.println("🛠️ Тип: Ручна архівація (без очищення history[])");
  }

 // if (!SPIFFS.exists("/log")) {
   // Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    //if (!SPIFFS.mkdir("/log")) {
     // Serial.println("❌ Не вдалося створити /log");
      //vTaskDelete(NULL);
     // return;
    //}
  //}

  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);

  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);  // UTF-8 BOM
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // оновлюємо лічильник
  lastSavedIndex = historyIndex;
  Serial.printf("📌 Оновлено lastSavedIndex → %d\n", lastSavedIndex);

  if (clearAfterSave) {
  clearHistoryData(); // очищення масиву поточних даних
  Serial.println("🧹 Масив history[] очищено після архівації");

  Serial.println("♻️ Виклик clearOldLogs(3)...");  // Додано
  clearOldLogs(3);  // очищення старих архівів
  Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
}


// Закоментовано для тестування блоку вище 
//if (clearAfterSave) {
//   clearHistoryData();
//   Serial.println("🧹 Масив history[] очищено (після автоматичної архівації)");

//clearOldLogs(3);  // видалення логів старших за 3 дні
//  Serial.println("🗑️ Старі архіви очищено (залишено останні 3 дні)");
//}

  vTaskDelete(NULL);
}

*/


/*
//Варіант від 19.07.2025 з додатковими повідомленнями коректно працює архівація і видалення старих архівів
// Не коректно працює перевірка наявності Директорії /log 
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.printf("➡️  Файл: %s\n", filename.c_str());

  if (clearAfterSave) {
    Serial.println("🔁 Тип: Автоматична архівація (з очищенням history[])");
  } else {
    Serial.println("🛠️ Тип: Ручна архівація (без очищення history[])");
  }

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);

  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);  // UTF-8 BOM
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // оновлюємо лічильник
  lastSavedIndex = historyIndex;
  Serial.printf("📌 Оновлено lastSavedIndex → %d\n", lastSavedIndex);

  if (clearAfterSave) {
  clearHistoryData(); // очищення масиву поточних даних
  Serial.println("🧹 Масив history[] очищено після архівації");

  Serial.println("♻️ Виклик clearOldLogs(3)...");  // Додано
  clearOldLogs(3);  // очищення старих архівів
  Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
}


  vTaskDelete(NULL);
}

*/

/*
//Варіант від 19.07.2025 з логікою, що виключає дублювання. НЕ перевірявся
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  // Підрахунок активних записів у history[]
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n",
        i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }

  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);
  if (nonEmpty == 0 || lastSavedIndex >= historyIndex) {
    Serial.println("⚠️ Нових записів для архівації немає.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF); // UTF-8 BOM
    file.println("Time,Temperature,Humidity,Pressure");
  }

  Serial.printf("📥 Архівація з рядка #%d до #%d...\n", lastSavedIndex, historyIndex - 1);
  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; ++i) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();
  file.close();
  delay(50); // Дати SPIFFS завершити

  Serial.printf("✅ Архів завершено: %s (%d нових записів)\n", filename.c_str(), written);

  // Оновлюємо межу архівації
  lastSavedIndex = historyIndex;
  Serial.printf("🆕 lastSavedIndex оновлено: %d\n", lastSavedIndex);

  if (clearAfterSave) {
    clearHistoryData();   // очищення масиву history[]
    historyIndex = 0;
    lastSavedIndex = 0;
    clearOldLogs(3);      // видалення старих архівів
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}

*/

/*
//попередній варіант не перевірявся!!!
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  // Вивід наявних записів для діагностики
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);

  if (nonEmpty == 0 || lastSavedIndex >= historyIndex) {
    Serial.println("⚠️ Немає нових записів для архівації.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Запис заголовка CSV, якщо файл порожній
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  Serial.printf("🔁 Архівація з рядка %d по %d...\n", lastSavedIndex, historyIndex - 1);

  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; ++i) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();   // 💾 Гарантований запис
  file.close();   // 🔐 Безпечне закриття
  delay(50);      // ⏳ Запас часу для SPIFFS

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // Оновлюємо межу архівації
  lastSavedIndex = historyIndex;
  Serial.printf("✅ Оновлено lastSavedIndex → %d\n", lastSavedIndex);

  if (clearAfterSave) {
    clearHistoryData(); // очищення масиву поточних даних
    clearOldLogs(3);    // 🔥 очищення архівів старших за 3 днів
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}
*/
//++++++++++++++++++++++++++++++++++++++
/*
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);
  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    }
    written++;
  }

  file.flush();   // 💾 Гарантований запис
  file.close();   // 🔐 Безпечне закриття
  delay(50);      // ⏳ Запас часу для SPIFFS

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData(); // очищення масиву поточних даних
    clearOldLogs(3);    // 🔥 очищення архівів старших за 3 днів
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}
*/

//++++++++++++++++++++++++++++++++++++++
/*
//=======+++++++++++++++++++++++
 //Проблема з повторним ручним архівуванням. При відкритті сторінкі архівів після ручного архівування її
 //необхідно оновити! Закоментовано 12_07_2025
//✅ saveHistoryTask() з діагностикою history[]:
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");

      vTaskDelete(NULL);
      return;
    }
  }

  // 🧪 Діагностика масиву history[]
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);
  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);  // ✅ ДОПИСУЄМО
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл новий — додати заголовок
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архівацію  завершено: %s (%d записів)\n", filename.c_str(), written);

  
  if (clearAfterSave) {
    clearHistoryData();        // очищення масиву поточних даних
    clearOldLogs(3);          // 🔥 очищення архівів старших за 3 днів
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}

*/

/* здається працює, але потребує перевірки
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;

  // Якщо переданий параметр (наприклад, false для ручного запуску)
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  File file = SPIFFS.open(filename, FILE_APPEND);  // ✅ ДОПИСУЄМО
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл новий і пустий — додаємо BOM + заголовок
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData();        // ✅ автоматично очищаємо
    clearOldLogs(2);          // ✅ і видаляємо старі архіви
  }

  vTaskDelete(NULL);
}

*/
/*
//Архівує, але при ручному виклику очищає масив поточних даних
void saveHistoryTask(void *parameter) {
  String filename = "/log/" + getTodayDate() + ".csv";

  // Діагностика
  Serial.println("📦 Запуск фонового архівування...");
  Serial.println("➡️  Файл: " + filename);

  // Створити директорію /log, якщо відсутня
  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);  // перезапис
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  file.write(0xEF); file.write(0xBB); file.write(0xBF);
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // Очищення масиву після завершення
  clearHistoryData();
  clearOldLogs(30);        // 🔧 ОЧИЩЕННЯ СТАРИХ АРХІВІВ ТУТ!
  vTaskDelete(NULL);
}
*/

/* //відключена (замінена) 10.07.2025
//Варіант GPT
void saveHistoryTask(void * parameter) {
  Serial.println("🟡 Початок фонового збереження історії...");

  String filename = "/log/" + getTodayDate() + ".csv";
  Serial.print("➡️ Спроба відкрити файл за шляхом: ");
  Serial.println(filename);

  // Створення директорії /log, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    Serial.println("[TASK] Директорія /log не знайдена, створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ [ERROR] Не вдалося створити директорію /log!");
      vTaskDelete(NULL);
      return;
    }
    Serial.println("✅ [TASK] Директорія /log успішно створена.");
  } else {
    Serial.println("[TASK] Директорія /log вже існує.");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);  // Перезапис файлу
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    vTaskDelete(NULL);
    return;
  }

  // UTF-8 BOM та заголовок CSV
  file.write(0xEF); file.write(0xBB); file.write(0xBF);
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (i % 50 == 0) yield();  // Не блокуємо інші задачі

    if (d.timeStr == "") continue;
    if (!d.timeStr.startsWith(getTodayDate())) continue;  // Фільтр лише для сьогоднішніх записів

    file.printf("%s,%.2f,%.2f,%.2f\n",
                d.timeStr.c_str(),
                d.temperature,
                d.humidity,
                d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Фонове архівування завершено: %s (%d записів)\n", filename.c_str(), written);

  vTaskDelete(NULL);
}
*/



/* не створює архів
void saveHistoryTask(void * parameter) {
  Serial.println("📁 Запускається фонове збереження історії...");

  String filename = String("/log_") + getTodayDate() + ".csv";
  Serial.print("🧾 Повний шлях до файлу: ");
  Serial.println(filename);

  // Відкриваємо файл для запису — SPIFFS дозволяє використовувати псевдо-директорії
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ [ERROR] Не вдалося відкрити файл! Можливо, недостатньо пам’яті або некоректний шлях.");
    vTaskDelete(NULL);
    return;
  }

  // Додаємо BOM для UTF-8 (Excel friendly)
  file.write(0xEF); file.write(0xBB); file.write(0xBF);
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (i % 50 == 0) {
      yield(); // Дозволяємо іншим задачам працювати
    }

    if (d.timeStr == "") {
      continue;
    }

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;

    if (written % 100 == 0) {
      Serial.printf("📥 Проміжний прогрес: %d записів\n", written);
    }
  }

  file.close();
  Serial.printf("✅ Історія збережена: %s (%d записів)\n", filename.c_str(), written);
  vTaskDelete(NULL);
}
*/
/* архів створюється, але виводить поточний файл
void saveHistoryTask(void * parameter) {
  Serial.println("Початок фонового збереження історії...");

  String filename = String("/log/") + getTodayDate() + ".csv";

  // Додано: Виведення повного шляху до файлу для діагностики
  Serial.print("Спроба відкрити файл за шляхом: ");
  Serial.println(filename);

  // Створюємо теку /log, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    Serial.println("[TASK] Директорія /log не знайдена, створюю...");
    if (!SPIFFS.mkdir("/log")) { // Перевіряємо, чи створення було успішним
      Serial.println("❌ [ERROR] Не вдалося створити директорію /log!");
      vTaskDelete(NULL); // Завершити завдання, якщо директорію не вдалося створити
      return;
    }
    Serial.println("✅ [TASK] Директорія /log успішно створена.");
  } else {
    // Додано: Повідомлення, якщо директорія вже існує (для підтвердження)
    Serial.println("[TASK] Директорія /log вже існує.");
  }


  File file = SPIFFS.open(filename, FILE_WRITE); // Перезапис
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    vTaskDelete(NULL); // Завершити завдання, якщо не вдалося відкрити файл
    return;
  }

  file.write(0xEF); file.write(0xBB); file.write(0xBF); // UTF-8 BOM
  file.println("Time,Temperature,Humidity,Pressure"); // Заголовок CSV

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (i % 50 == 0) {
      yield(); // Дозволити іншим завданням працювати
    }

    if (d.timeStr == "") {
      continue; // пропустити порожні записи
    }

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Фонове архівування завершено: %s (%d записів)\n", filename.c_str(), written);

  vTaskDelete(NULL); // Завершити завдання після його виконання
}
*/
//==============+++++++++++++++++++++++++ 

/*
// --- NEW FreeRTOS Task for saving history ---
// Замінює стару функцію saveHistoryToFile()
void saveHistoryTask(void * parameter) {
  Serial.println("Початок фонового збереження історії...");

  String filename = String("/log/") + getTodayDate() + ".csv";

  // Створюємо теку /log, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
    Serial.println("Директорія /log  створена в друге!");
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
 //==============+++++++++++++++++++++++++ 
*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Функція для видалення конкретного файлу з архіву
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
//Редакція №4
void deleteLogFile(const String& fullPath) {
  Serial.println("📂 Запит на видалення архіву: " + fullPath);

  if (SPIFFS.exists(fullPath)) {
    Serial.println("✅ Файл існує. Проводжу видалення...");
    if (SPIFFS.remove(fullPath)) {
      Serial.println("🗑️ Файл успішно видалено: " + fullPath);
    } else {
      Serial.println("❌ ПОМИЛКА: Не вдалося видалити файл: " + fullPath);
    }
  } else {
    Serial.println("⚠️ Файл не знайдено: " + fullPath);
  }
}
*/
/*
//Редакція №3
void deleteLogFile(const String& filename) {
  String path = filename;
  if (!path.startsWith("/")) {
    path = "/" + path;
  }

  Serial.println("📂 Запит на видалення архіву: " + path);

  if (SPIFFS.exists(path)) {
    Serial.println("✅ Файл існує. Проводжу видалення...");
    if (SPIFFS.remove(path)) {
      Serial.println("🗑️ Файл успішно видалено: " + path);
    } else {
      Serial.println("❌ ПОМИЛКА: Не вдалося видалити файл: " + path);
    }
  } else {
    Serial.println("⚠️ Файл не знайдено: " + path);
  }
}
*/
/*
//Редакція №2
void deleteLogFile(const char* filename) {
  String fullPath = "/";
  fullPath += filename;

  Serial.println("📂 Запит на видалення архіву: " + fullPath);

  if (SPIFFS.exists(fullPath)) {
    Serial.println("✅ Файл існує. Проводжу видалення...");

    if (SPIFFS.remove(fullPath)) {
      Serial.println("🗑️ Файл успішно видалено: " + fullPath);
    } else {
      Serial.println("❌ ПОМИЛКА: Не вдалося видалити файл: " + fullPath);
    }
  } else {
    Serial.println("⚠️ Файл не знайдено: " + fullPath);
  }
}
*/

//Редакція №1
//Видалення із архіву конкретного файлу ПРАЦЮЄ. Приклад deleteLogFile("2025-07-10.csv");
void deleteLogFile(const char* filename) {
  String fullPath = "/log/";
  fullPath += filename;

  Serial.println("📂 Запит на видалення архіву: " + fullPath);

  if (SPIFFS.exists(fullPath)) {
    Serial.println("✅ Файл існує. Проводжу видалення...");

    if (SPIFFS.remove(fullPath)) {
      Serial.println("🗑️ Файл успішно видалено: " + fullPath);
    } else {
      Serial.println("❌ ПОМИЛКА: Не вдалося видалити файл: " + fullPath);
    }
  } else {
    Serial.println("⚠️ Файл не знайдено: " + fullPath);
  }

}

  //Варіант від 20.07.2025 Не перевірявся
/*  
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити директорію /log.");
    return;
  }

  time_t now;
  time(&now);

  int totalFiles = 0, deletedFiles = 0;

  File file = root.openNextFile();
  while (file) {
    String path = file.name();  // /log/2025-07-10.csv
    totalFiles++;

    Serial.printf("🔍 Знайдено файл: %s (%u байт)\n", path.c_str(), file.size());

    if (!path.endsWith(".csv")) {
      Serial.println("⚠️ Пропускаємо: не .csv файл.");
      file = root.openNextFile();
      continue;
    }

    // Витягуємо ім’я файлу без шляху
    String filename = path.substring(path.lastIndexOf('/') + 1); // 2025-07-10.csv

    if (filename.length() != 14) {
      Serial.println("⚠️ Ім’я файлу має неочікувану довжину — пропуск.");
      file = root.openNextFile();
      continue;
    }

    // Витягуємо дату
    int year  = filename.substring(0, 4).toInt();
    int month = filename.substring(5, 7).toInt();
    int day   = filename.substring(8, 10).toInt();

    if (year == 0 || month == 0 || day == 0) {
      Serial.println("⚠️ Не вдалося розпізнати дату в імені — пропуск.");
      file = root.openNextFile();
      continue;
    }

    struct tm fileDate = {};
    fileDate.tm_year = year - 1900;
    fileDate.tm_mon  = month - 1;
    fileDate.tm_mday = day;
    fileDate.tm_hour = 0;
    fileDate.tm_min  = 0;
    fileDate.tm_sec  = 0;

    time_t fileEpoch = mktime(&fileDate);
    double ageDays = difftime(now, fileEpoch) / 86400.0;

    Serial.printf("📅 Вік файлу: %.2f днів (%s)\n", ageDays, filename.c_str());

    if (ageDays > maxDays) {
      Serial.printf("🗑 Видаляю файл: %s\n", path.c_str());
      SPIFFS.remove(path);
      deletedFiles++;
    } else {
      Serial.println("✅ Файл актуальний — не видаляється.");
    }

    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", totalFiles, deletedFiles);
  Serial.println("♻️ Очищення завершено.");
}
*/

//Оновлена 15_07_2025
//Варіант №12
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
    return;
  }

  time_t now;
  time(&now);

  int totalFiles = 0, deletedFiles = 0;

  File file = root.openNextFile();
  while (file) {
    String fullPath = file.name();         // напр. "/2025-07-10.csv"
    size_t size = file.size();
    totalFiles++;

    Serial.printf("🔍 Знайдено файл: %s (%u байт)\n", fullPath.c_str(), size);

    if (!fullPath.endsWith(".csv")) {
      Serial.println("⚠️ Пропускаємо: не .csv файл.");
      file = root.openNextFile();
      continue;
    }

    // Витягуємо лише ім’я файлу без шляху "/"
    String fileNameOnly = fullPath.substring(fullPath.lastIndexOf('/') + 1);
    fileNameOnly.trim();

    Serial.printf("📄 Ім’я для аналізу: [%s] (довжина: %d)\n", fileNameOnly.c_str(), fileNameOnly.length());

    // Покажемо побайтово
    for (size_t i = 0; i < fileNameOnly.length(); i++) {
      Serial.printf("  char[%d] = '%c' (0x%02X)\n", i, fileNameOnly[i], fileNameOnly[i]);
    }

    struct tm tm = {};
    if (strptime(fileNameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;

      Serial.printf("📅 Вік файлу: %.2f днів\n", ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Видалення файлу: %s (вік: %.1f днів)\n", fileNameOnly.c_str(), ageDays);
        deleteLogFile(fileNameOnly.c_str());  // ← правильне ім’я
        deletedFiles++;
      } else {
        Serial.println("✅ Файл актуальний — не видаляється.");
      }
    } else {
      Serial.println("⚠️ Неможливо розпарсити дату з імені.");
    }

    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", totalFiles, deletedFiles);
  Serial.println("♻️ Очищення завершено.");
}

/*
//Варіант №11 Не працює. Не вірно визначає "Пропускаємо: не .csv файл або не в корені.""
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів   старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
    return;
  }

  File file = root.openNextFile();
  int total = 0, deleted = 0;

  time_t now;
  time(&now);

  while (file) {
    String name = file.name();
    size_t size = file.size();
    file.close();
    total++;

    Serial.printf("🔍 Знайдено файл: %s (%d байт)\n", name.c_str(), (int)size);

    // Перевірка чи це .csv-файл у корені
    if (!name.endsWith(".csv") || name.indexOf('/') != 0 || name.lastIndexOf('/') > 0) {
      Serial.println("⚠️ Пропускаємо: не .csv файл або не в корені.");
      file = root.openNextFile();
      continue;
    }

    String fileNameOnly = name;
    fileNameOnly.remove(0, 1);  // видаляємо початковий слеш, отримаємо "2025-07-11.csv"

    struct tm tm = {};
    if (strptime(fileNameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;
      Serial.printf("📄 Ім’я для аналізу: %s\n", fileNameOnly.c_str());
      Serial.printf("📅 Вік файлу: %.2f днів\n", ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Спроба   видалити файл: %s (вік: %.1f днів)\n", name.c_str(), ageDays);
        deleteLogFile(name);  // ← важливо: name вже містить повний шлях "/2025-07-11.csv"
        deleted++;
      } else {
        Serial.println("✅ Файл актуальний — не видаляється.");
      }

    } else {
      Serial.println("⚠️ Не вдалося розпарсити дату з імені файлу.");
    }

    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", total, deleted);
  Serial.println("♻️ Очищення завершено.");
}
*/
/*
//Варіант №10
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів  старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
    return;
  }

  File file = root.openNextFile();
  int totalFiles = 0, deletedFiles = 0;

  time_t now;
  time(&now);

  while (file) {
    String name = file.name();  // наприклад: /2025-07-11.csv
    size_t size = file.size();
    file.close();
    totalFiles++;

    Serial.printf("🔍 Знайдено файл: %s (%d байт)\n", name.c_str(), size);

    if (!name.endsWith(".csv")) {
      Serial.println("⚠️ Пропускаємо: не .csv файл.");
      file = root.openNextFile();
      continue;
    }

    // Витягуємо лише ім’я файлу без шляху
    String fileNameOnly = name;
    if (fileNameOnly.startsWith("/")) fileNameOnly.remove(0, 1);

    struct tm tm = {};
    if (strptime(fileNameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;

      Serial.printf("📄 Ім’я для аналізу: %s\n", fileNameOnly.c_str());
      Serial.printf("📅 Вік файлу: %.2f днів\n", ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Спроба  видалити файл: %s (вік: %.1f днів)\n", fileNameOnly.c_str(), ageDays);
        deleteLogFile(fileNameOnly);  // ← передаємо без "/"
        deletedFiles++;
      } else {
        Serial.println("✅ Файл актуальний — не видаляється.");
      }
    } else {
      Serial.println("⚠️ Неможливо розпарсити дату з імені.");
    }

    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", totalFiles, deletedFiles);
  Serial.println("♻️ Очищення завершено.");
}
*/
/*
//Варіант №9
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів  старших за %d днів...\n", maxDays);
  
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Помилка: коренева директорія SPIFFS недоступна або не є директорією.");
    return;
  }

  File file = root.openNextFile();
  int totalChecked = 0;
  int totalDeleted = 0;

  time_t now;
  time(&now);

  while (file) {
    String path = file.name(); // напр. "/2025-07-10.csv"
    size_t size = file.size();
    file.close();

    Serial.printf("🔍 Знайдено файл: %s (%u байт)\n", path.c_str(), (unsigned)size);

    // Перевірка: чи це CSV-файл
    if (!path.endsWith(".csv")) {
      Serial.println("⚠️ Пропускаємо: не .csv файл.");
      file = root.openNextFile();
      continue;
    }

    // Вилучення імені файлу без шляху
    String nameOnly = path;
    if (nameOnly.startsWith("/")) nameOnly = nameOnly.substring(1);  // "2025-07-10.csv"

    Serial.println("📄 Ім’я для аналізу: " + nameOnly);

    // Парсинг дати з імені файлу
    struct tm tm = {};
    if (strptime(nameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;
      Serial.printf("📅 Вік файлу: %.2f днів\n", ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Спроба видалити файл: %s (вік: %.1f днів)\n", nameOnly.c_str(), ageDays);
        deleteLogFile(nameOnly.c_str());
        totalDeleted++;
      } else {
        Serial.println("✅ Файл актуальний — не видаляється.");
      }

      totalChecked++;
    } else {
      Serial.println("❌ Не вдалося розпізнати дату з імені файлу.");
    }

    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", totalChecked, totalDeleted);
  Serial.println("♻️ Очищення завершено.");
}
*/
/*
//Варіант №8
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
    return;
  }

  File file = root.openNextFile();
  int totalChecked = 0;
  int totalDeleted = 0;

  while (file) {
    String path = file.name();
    size_t fileSize = file.size();
    file.close();

    Serial.printf("🔍 Знайдено файл: %s (%u байт)\n", path.c_str(), (unsigned)fileSize);

    // Перевіряємо, що файл має формат /YYYY-MM-DD.csv
    if (!path.endsWith(".csv") || path.length() < 15) {
      Serial.println("⚠️ Пропускаємо: не схоже на архівний .csv файл.");
      file = root.openNextFile();
      continue;
    }

    // Витягуємо лише ім’я файлу (без шляху)
    String nameOnly = path.substring(path.lastIndexOf('/') + 1);  // "2025-07-10.csv"
    Serial.println("📄 Ім’я для аналізу: " + nameOnly);

    struct tm tm = {};
    if (!strptime(nameOnly.c_str(), "%Y-%m-%d.csv", &tm)) {
      Serial.println("❌ Не вдалося розпарсити дату.");
      file = root.openNextFile();
      continue;
    }

    time_t fileTime = mktime(&tm);
    time_t now;
    time(&now);

    double ageDays = difftime(now, fileTime) / 86400.0;
    Serial.printf("📅 Вік файлу: %.2f днів\n", ageDays);

    if (ageDays > maxDays) {
      Serial.printf("🗑 Спроба видалити файл: %s (age: %.2f)\n", nameOnly.c_str(), ageDays);
      deleteLogFile(nameOnly.c_str());  // ← Ім’я без шляху!
      totalDeleted++;
    } else {
      Serial.println("✅ Файл актуальний — не видаляється.");
    }

    totalChecked++;
    file = root.openNextFile();
  }

  Serial.printf("🔎 Перевірено %d файлів, видалено %d архівів.\n", totalChecked, totalDeleted);
}
*/
/*
//Варіант №7
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів   старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію.");
    return;
  }

  int totalChecked = 0;
  int totalDeleted = 0;

  File file = root.openNextFile();
  while (file) {
    String path = file.name();
    size_t fileSize = file.size();
    file.close();

    if (!path.endsWith(".csv") || path.indexOf('/') != 0) {
      file = root.openNextFile();
      continue;
    }

    // Витягуємо ім’я, напр. "2025-07-10.csv"
    String name = path.substring(path.lastIndexOf("/") + 1);
    struct tm tm = {};
    if (strptime(name.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      time_t now;
      time(&now);
      double age = difftime(now, fileTime) / 86400.0;

      Serial.printf("📄 [%s] (%d байт) - %.1f днів\n", name.c_str(), fileSize, age);

      if (age > maxDays) {
        deleteLogFile(name.c_str());
        totalDeleted++;
      } else {
        Serial.println("ℹ️ Актуальний файл — не видаляється.");
      }
    } else {
      Serial.printf("⚠️ Невідомий формат імені: %s\n", name.c_str());
    }

    totalChecked++;
    file = root.openNextFile();
  }

  Serial.printf("✅ Перевірено %d файлів, видалено %d старих архівів.\n", totalChecked, totalDeleted);
  Serial.println("♻️ Очищено архів. Старші " + String(maxDays) + " днів видалено");
}
*/
/*
//Варіант №6
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Неможливо відкрити кореневу директорію.");
    return;
  }

  File file = root.openNextFile();
  int totalChecked = 0, totalDeleted = 0;

  time_t now;
  time(&now);

  while (file) {
    String path = file.name();
    size_t fsize = file.size();
    file.close();
    totalChecked++;

    // Ігноруємо всі файли, які не .csv
    if (!path.endsWith(".csv")) {
      file = root.openNextFile();
      continue;
    }

    // Отримуємо тільки ім’я файлу (без "/")
    String name = path;
    name.replace("/", "");

    // Парсимо дату
    struct tm tm = {};
    if (strptime(name.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;

      Serial.printf("📄 [%s] (%u байт) - %.1f днів\n", name.c_str(), (unsigned int)fsize, ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Спроба видалити [%s]... ", path.c_str());
        if (SPIFFS.remove(path)) {
          Serial.println("✅ Успішно.");
          totalDeleted++;
        } else {
          Serial.println("❌ Помилка видалення.");
        }
      } else {
        Serial.println("ℹ️ Актуальний файл — не видаляється.");
      }
    } else {
      Serial.printf("⚠️ Пропущено: не вдалося розпізнати дату в [%s]\n", name.c_str());
    }

    file = root.openNextFile();
  }

  Serial.printf("✅ Перевірено %d файлів, видалено %d старих архівів.\n", totalChecked, totalDeleted);
}
*/
/*
//Варіант №5
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів  старших за %d днів...\n", maxDays);
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Неможливо відкрити кореневу директорію SPIFFS.");
    return;
  }

  File file = root.openNextFile();
  int totalFiles = 0;
  int deletedCount = 0;

  time_t now;
  time(&now);

  while (file) {
    String path = file.name();
    size_t size = file.size();
    file.close();
    totalFiles++;

    if (!path.endsWith(".csv")) {
      file = root.openNextFile();
      continue;
    }

    // Діагностика: вивід шляху файлу
    Serial.printf("📄 [%s] (%u байт)", path.c_str(), (unsigned int)size);

    // Витягнути лише ім’я файлу без шляху
    String name = path;
    if (name.startsWith("/")) name.remove(0, 1); // прибрати початковий '/'
    String datePart = name;
    datePart.replace(".csv", "");  // прибрати розширення

    struct tm tm = {};
    if (!strptime(datePart.c_str(), "%Y-%m-%d", &tm)) {
      Serial.println(" ⚠️ Не вдалося розпізнати дату.");
      file = root.openNextFile();
      continue;
    }

    time_t fileTime = mktime(&tm);
    double ageDays = difftime(now, fileTime) / 86400.0;

    Serial.printf(" - %.1f днів\n", ageDays);

    if (ageDays > maxDays) {
      if (!path.startsWith("/")) path = "/" + path;  // переконатися в наявності '/'
      Serial.printf("🗑 Спроба видалити [%s]... ", path.c_str());
      if (SPIFFS.exists(path)) {
        if (SPIFFS.remove(path)) {
          Serial.println("✅ Видалено.");
          deletedCount++;
        } else {
          Serial.println("❌ ПОМИЛКА видалення.");
        }
      } else {
        Serial.println("⚠️ Файл не знайдено.");
      }
    } else {
      Serial.println("ℹ️ Актуальний файл — не видаляється.");
    }

    file = root.openNextFile();
  }

  Serial.printf("✅ Перевірено %d файлів, видалено %d старих архівів.\n", totalFiles, deletedCount);
}

*/

/*
//Варіант №4
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
    return;
  }

  time_t now;
  time(&now);

  int deletedCount = 0;
  int scanned = 0;

  File file = root.openNextFile();
  while (file) {
    String path = file.name();
    size_t size = file.size();
    file.close();

    scanned++;

    // Шукаємо лише файли CSV з іменами YYYY-MM-DD.csv
    if (!path.endsWith(".csv")) {
      file = root.openNextFile();
      continue;
    }

    // Витягуємо тільки ім’я файлу без шляху
    String filename = path.substring(path.lastIndexOf("/") + 1);

    struct tm tm = {};
    if (strptime(filename.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;

      Serial.printf("📄 %s (%d байт) - %.1f днів\n", filename.c_str(), (int)size, ageDays);

      if (ageDays > maxDays) {
        Serial.printf("🗑 Видаляю %s ... ", path.c_str());
        if (SPIFFS.remove(path)) {
          Serial.println("✅ Видалено.");
          deletedCount++;
        } else {
          Serial.println("❌ Не вдалося видалити.");
        }
      } else {
        Serial.println("ℹ️ Актуальний файл — не видаляється.");
      }
    } else {
      Serial.println("⚠️ Не вдалося визначити дату: " + filename);
    }

    file = root.openNextFile();
  }

  Serial.printf("✅ Перевірено %d файлів, видалено %d старих архівів.\n", scanned, deletedCount);
}
*/
/*
//Варіант №3
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);
  
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("❌ Не вдалося відкрити кореневу теку SPIFFS");
    return;
  }

  time_t now;
  time(&now);

  File file = root.openNextFile();
  while (file) {
    String path = file.name();
    file.close();  // Закриваємо одразу

    if (!path.startsWith("/log/") || !path.endsWith(".csv")) {
      file = root.openNextFile();
      continue;
    }

    // Вилучаємо "2025-07-10.csv" з "/log/2025-07-10.csv"
    String fileName = path.substring(5); // довжина "/log/" = 5

    struct tm tm = {};
    if (strptime(fileName.c_str(), "%Y-%m-%d.csv", &tm)) {
      time_t fileTime = mktime(&tm);
      double ageDays = difftime(now, fileTime) / 86400.0;

      if (ageDays > maxDays) {
        Serial.printf("🕓 Файл %s старший за %d днів (%.1f днів)\n", path.c_str(), maxDays, ageDays);
        if (SPIFFS.remove(path)) {
          Serial.println("✅ Видалено: " + path);
        } else {
          Serial.println("❌ Не вдалося видалити: " + path);
        }
      }
    } else {
      Serial.println("⚠️ Неможливо розпарсити дату з імені: " + fileName);
    }

    file = root.openNextFile();
  }
}
*/
//========================================

/*
//Варіант №2
void clearOldLogs(int maxDays) {
  Serial.printf("🧹 Очищення логів старших за %d днів...\n", maxDays);

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити кореневу директорію.");
    return;
  }

  time_t now;
  time(&now);

  File file = root.openNextFile();
  while (file) {
    String name = file.name(); // Наприклад: /2025-07-10.csv
    file.close();

    // Перевірка формату: /YYYY-MM-DD.csv (довжина 16 символів)
    if (name.startsWith("/") && name.endsWith(".csv") && name.length() == 16) {
      String fileNameOnly = name.substring(1);  // "2025-07-10.csv"

      int yyyy = fileNameOnly.substring(0, 4).toInt();
      int mm   = fileNameOnly.substring(5, 7).toInt();
      int dd   = fileNameOnly.substring(8, 10).toInt();

      if (yyyy > 2000 && mm >= 1 && mm <= 12 && dd >= 1 && dd <= 31) {
        struct tm tm = {};
        tm.tm_year = yyyy - 1900;
        tm.tm_mon  = mm - 1;
        tm.tm_mday = dd;
        tm.tm_hour = 0;

        time_t fileTime = mktime(&tm);
        double ageDays = difftime(now, fileTime) / 86400.0;

        if (ageDays > maxDays) {
          if (SPIFFS.remove(name)) {
            Serial.println("🗑️ Видалено файл: " + name);
          } else {
            Serial.println("⚠️ Не вдалося видалити файл: " + name);
          }
        }
      }
    }

    file = root.openNextFile();
  }
}
*/

//+++++++++++++
/*
// Не видаляє старі архіви Варіант №1
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
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ==== Реалізація функції clearHistoryData() ====
// Розмістіть цей блок коду в одному з ваших .cpp файлів (наприклад, main.cpp або subroutines.cpp).
// Ця функція очищує масив history, обнуляючи всі його записи.
//Оновлено 19.07.2025
void clearHistoryData() {
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    history[i] = BMEData();  // Обнуляємо структуру
  }
  Serial.println("🧽 Масив history[] очищено");
}

/*
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
//+++++++++++++
*/
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
//+++++++++++++

 Serial.println("Ініціалізація SPIFFS...");
  if (!SPIFFS.begin()) {
    Serial.println("❌ Помилка при монтуванні SPIFFS");
    // Якщо SPIFFS не може змонтуватися, спробуйте відформатувати лише один раз
    // Однак, ми вже виключили це як регулярну проблему.
    // Якщо тут завжди помилка, це серйозніше.
    while (true) { delay(100); } // Застрягти тут, якщо SPIFFS не монтується
  }
  Serial.println("✅ SPIFFS успішно змонтовано.");

  resetHistoryForNewDay(); // скидаємо індекс історії на нуль
   Serial.println("✅ Індекс історії успішно очищено.");
  //=========================
  //Видалення конкретного лог-файлу
        //deleteLogFile("2025-07-10.csv");
  //=========================
  // ==== Ваш існуючий код для створення директорії /log ====
  // Перевіряємо та створюємо директорію /log, якщо її немає
  if (!SPIFFS.exists("/log")) {
    Serial.println("[SETUP] Директорія /log не знайдена, створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ [ERROR] Не вдалося створити директорію /log!");
      // Якщо директорію не вдалося створити, ми не зможемо зберегти логи
      while (true) { delay(100); } // Застрягти тут
    }
    Serial.println("✅ [SETUP] Директорія /log успішно створена.");
  } else {
    Serial.println("[SETUP] Директорія /log вже існує.");
  }
  // =========================================================

  // ==== НОВИЙ ДІАГНОСТИЧНИЙ КОД ====
  Serial.println("Перевіряю вміст кореневої директорії SPIFFS:");
  File root = SPIFFS.open("/");
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    int fileCount = 0;
    while (file) {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      if (file.isDirectory()) {
        Serial.println(" (DIRECTORY)");
      } else {
        Serial.print(" (FILE, size: ");
        Serial.print(file.size());
        Serial.println(" bytes)");
      }
      file = root.openNextFile();
      fileCount++;
    }
    if (fileCount == 0) {
      Serial.println("  Коренева директорія порожня.");
    }
  } else {
    Serial.println("❌ Не вдалося відкрити кореневу директорію SPIFFS.");
  }
  Serial.println("=========================================");
  // ===================================


//+++++++++++++++++++++
  bme.begin(0x76);

  // ІНІЦІАЛІЗАЦІЯ МАСИВУ ІСТОРІЇ
  for (int j = 0; j < MAX_MEASUREMENTS; ++j) {
      history[j].timeStr = "";
      history[j].temperature = 0.0f;
      history[j].humidity = 0.0f;
      history[j].pressure = 0.0f;
  }
  
  loadConfig();
  //++++++++++++++++++++++++++++++++++++++
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();
  //++++++++++++++++++++++++++++++++++++++
  WiFi.begin("Smart_House", "Telemat5311051");
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
  delay(3000);
// === НОВЕ: Перше зчитування даних BME280 одразу після запуску ===
    // Цей блок коду заповнює lastTemp, lastHum, lastPress актуальними значеннями.
    lastTemp = bme.readTemperature();
    lastHum = bme.readHumidity();
    lastPress = bme.readPressure(); // lastPress зберігається в Паскалях

    Serial.printf("Initial BME280: Temp=%.1fC, Hum=%.1f%%, Press=%.1fPa\n", lastTemp, lastHum, lastPress);

    // У вашій функції setup() знайдіть цей рядок:
configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // <-- ЗМІНІТЬ ЦЕ

// НА ЦЕ:
//configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Дозволяє setenv("TZ", ...) повністю керувати часовим поясом
 
  

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  }
  //++++++++++++++++++++++++++++++++++++++
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  //++++++++++++++++++++++++++++++++++++++
  server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/graph.js", "application/javascript");
  });

  //++++++++++++++++++++++++++++++++++++++
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
    //++++++++++++++++++++++++++++++++++++++
    // Забезпечуємо правильне завантаження файлу CSV
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, "text/csv");
    response->addHeader("Content-Disposition", "attachment; filename=\"" + date + ".csv\"");
    request->send(response);
  });

  //++++++++++++++++++++++++++++++++++++++
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });

  //++++++++++++++++++++++++++++++++++++++
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

  //++++++++++++++++++++++++++++++++++++++
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  //++++++++++++++++++++++++++++++++++++++
  server.on("/bme.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bme.html", "text/html; charset=utf-8");
  });

  //++++++++++++++++++++++++++++++++++++++
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

  //++++++++++++++++++++++++++++++++++++++
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
  //++++++++++++++++++++++++++++++++++++++
  // Виведення архівних графіків
  server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (!request->hasParam("date")) {
    request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
    return;
  }

  String date = request->getParam("date")->value();
  String path = "/log/" + date + ".csv";

  if (!SPIFFS.exists(path)) {
    request->send(404, "application/json", "[]");  // Порожній графік
    return;
  }

  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    request->send(500, "application/json", "[]");
    return;
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print("[");
  bool first = true;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line == "" || line.startsWith("Time")) continue;

    int idx1 = line.indexOf(',');
    int idx2 = line.indexOf(',', idx1 + 1);
    int idx3 = line.indexOf(',', idx2 + 1);

    if (idx1 < 0 || idx2 < 0 || idx3 < 0) continue;

    String timeStr = line.substring(0, idx1);
    float temp = line.substring(idx1 + 1, idx2).toFloat();
    float hum = line.substring(idx2 + 1, idx3).toFloat();
    float press = line.substring(idx3 + 1).toFloat();

    if (!first) response->print(",");
    first = false;

    response->printf(
      "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
      timeStr.c_str(), temp, hum, press
    );
  }

  response->print("]");
  request->send(response);
});
//++++++++++++Динамічний масштаб++++++++++
//15.07.2025. Не перевірено
server.on("/api/history.json", HTTP_GET, [](AsyncWebServerRequest *request){
  File file = SPIFFS.open("/log/2025-07-14.csv");  // поточна дата
  if (!file || file.isDirectory()) {
    request->send(404, "application/json", "{\"error\":\"Файл не знайдено\"}");
    return;
  }

  DynamicJsonDocument doc(8192); // або більший розмір
  JsonArray arr = doc.to<JsonArray>();

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    // CSV: time;temp;hum;pres
    int t1 = line.indexOf(';');
    int t2 = line.indexOf(';', t1 + 1);
    int t3 = line.indexOf(';', t2 + 1);

    if (t1 > 0 && t2 > t1 && t3 > t2) {
      String time = line.substring(0, t1);
      float temp = line.substring(t1 + 1, t2).toFloat();
      float hum  = line.substring(t2 + 1, t3).toFloat();
      float pres = line.substring(t3 + 1).toFloat();

      JsonObject obj = arr.createNestedObject();
      obj["time"] = time;
      obj["temperature"] = temp;
      obj["humidity"] = hum;
      obj["pressure"] = pres;
    }
  }

  file.close();

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
});

  //++++++++++++++++++++++++++++++++++++++
  /*
//Версія від 14.07.2025
server.on("/log_list.json", HTTP_GET, [](AsyncWebServerRequest *request){
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  String json = "[";
  bool first = true;

  while (file) {
    String path = file.name();
    file.close();

    if (!path.endsWith(".csv") || !path.startsWith("/log/")) {
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
*/
/*
//викликає помилки при компіляції
server.on("/log_list.json", HTTP_GET, [](AsyncWebServerRequest *request) {
  JsonDocument doc; // Створюємо JSON документ
  JsonArray filesArray = doc.to<JsonArray>(); // Створюємо JSON масив для файлів

  File dir = SPIFFS.open("/log"); // Правильно: відкриваємо директорію /log
  if (!dir || !dir.isDirectory()) {
    Serial.println("❌ /log_list.json: Не вдалося відкрити директорію /log або вона не існує.");
    request->send(200, "application/json", "[]"); // Відправляємо порожній масив, якщо директорія недоступна
    return;
  }

  File file = dir.openNextFile();
  while (file) {
    if (!file.isDirectory()) { // Перевіряємо, що це файл, а не вкладена директорія
      String fileName = file.name(); // Отримаємо лише ім'я файлу, наприклад "2025-07-15.csv"
      
      if (fileName.endsWith(".csv")) { // Фільтруємо за розширенням .csv
        JsonObject item = filesArray.add<JsonObject>(); // Додаємо новий об'єкт до масиву
        item["name"] = fileName; // Ім'я файлу, наприклад "2025-07-15.csv"
        item["path"] = "/log/" + fileName; // <--- ОСЬ ТУТ КЛЮЧОВА ЗМІНА: формуємо ПОВНИЙ ШЛЯХ для браузера
      }
    }
    file.close(); // Завжди закриваємо поточний об'єкт File
    file = dir.openNextFile(); // Переходимо до наступного запису
  }
  dir.close(); // Закриваємо директорію після перебору

  String output;
  serializeJson(doc, output); // Серіалізуємо JSON документ у рядок
  request->send(200, "application/json", output); // Відправляємо JSON-відповідь
});
*/
  
  // не повний шлях до файлу .csv 15.07.2025
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
  
  //===============================================================================
  //Версія від 14.07.2025
  /*
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request){
  String html = "<h2>Доступні архіви:</h2><ul>";

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    html += "<li>Немає доступних лог-файлів.</li>";
  } else {
    File file = root.openNextFile();
    while (file) {
      String path = file.name();
      file.close();

      if (!path.endsWith(".csv") || !path.startsWith("/log/")) {
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
*/
  
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
  
 //++++++++++++++++++++++++++++++++++++++
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });

  //++++++++++++++++++++++++++++++++++++++
  
  // Працює. На кнопкі Ок абракадабра
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

      //request->send(200, "text/html",
      request->send(200, "text/html; charset=utf-8",
        "<script>alert('Налаштування збережено!');location.href='/settings';</script>");
  });
 
  //++++++++++++++++++++++++++++++++++++++
  //Заміна 19.07.2025
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
  bool *param = new bool(false);  // ❗ ручне збереження без очищення масиву
  xTaskCreatePinnedToCore(
    saveHistoryTask,
    "SaveHistory",
    8192,
    param,
    1,
    NULL,
    1
  );
  request->send(200, "text/plain", "✅ Ручне збереження розпочато у фоновому режимі.");
});


/*

server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
  bool *param = new bool(false);  // ❌ не очищати history
  xTaskCreate(
    saveHistoryTask,
    "SaveHistory",
    8192,
    param,
    1,
    NULL
  );
  request->send(200, "text/plain", "✅ Ручне збереження розпочато у фоновому режимі.");
});
*/
/*
//закоментовано 10.07.2025
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
  */
  //++++++++++++++++++++++++++++++++++++++
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

  //++++++++++++++++++++++++++++++++++++++
  // --- UPDATED onNotFound handler ---
  server.onNotFound([](AsyncWebServerRequest *request){
    // Відправляємо HTML-сторінку 404 з коректним кодуванням UTF-8
    request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
  });
  //++++++++++++++++++++++++++++++++++++++
  // Обробка запитів для файлів логів з директорії /log/
  // URL-шлях "/log/" відповідає директорії "/log/" на SPIFFS
  server.serveStatic("/log/", SPIFFS, "/log/"); 
  //++++++++++++++++++++++++++++++++++++++
  // Запуск веб-сервера
  server.begin();
     currentDisplayMode = MODE_DATETIME; // Або якийсь інший режим за замовчуванням
     executeDisplayMode(currentDisplayMode); // Відобразити початковий режим
     displayModeStartTime = millis(); // Запустити таймер для цього режиму
}

void loop() {
  unsigned long currentMillis = millis();
  static bool archiveDoneToday = false;
  static bool fileCreated = false;
  static String lastInitDate = "";
  String timeOnly = getShortTime();  // поверне "HH:MM"
  String currentTime = getCurrentTime();   // "YYYY-MM-DD-HH:MM:SS" 
  String today = getTodayDate();           // "YYYY-MM-DD"

  today = getTodayDate();
  //String timeOnly = getCurrentTime().substring(11);  // HH:MM:SS → HH:MM

  /*
//Test
if (timeOnly >= "09:10" && flag_test){
 clearOldLogs(3);
  Serial.println("♻️ Очищено архів. Старші 3 днів видалено");
 flag_test=0; 
}

//End Test
*/
/*
 // 🧪 Тестовий автозапуск архівації
  //String timeOnly = getCurrentTime().substring(11); // "HH:MM:SS"
  if (timeOnly >= "15:10" && testArchiveFlag) {
    Serial.println("🧪 Тестовий запуск архівації з очищенням архівів...");

    bool *param = new bool(true);  // true = автоматична архівація
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    testArchiveFlag = false;
  }

  //End Test
*/
// ♻️ Скидання флагів один раз на початку нової доби (після 00:01 і до 23:58)
static String lastDateReset = "";
if (timeOnly >= "00:01" && timeOnly < "23:58" && today != lastDateReset) {
  Serial.println("♻️ Скидання флагів архівації та створення — нова дата!");
  archiveDoneToday = false;
  fileCreated = false;
  lastDateReset = today;
  resetHistoryForNewDay(); // скидаємо індекс історії на нуль
}
/*
// currentTime замінено на timeOnly
if (currentTime >= "00:01" && currentTime < "23:58" && today != lastDateReset) {
  Serial.println("♻️ Скидання флагів архівації та створення — нова дата!");
  archiveDoneToday = false;
  fileCreated = false;
  lastDateReset = today;
}

//Test
if ((timeOnly >= "13:00") && (timeOnly <="13:02")) {
 Serial.println("🕓 Поточний час >= 13:00 — тест пройдено..."); 
}
if (timeOnly >= "13:01" && timeOnly <="13:03") {
 Serial.println("🕓 Поточний час >= 13:01 — функція працює"); 
} 

if (timeOnly >= "13:05" && !archiveDoneToday) {
    Serial.println("🕓 Поточний час >= 13:05 — умови запуску команди виконано");
}
*/
//////////////////////////////

//Оновлено 19.07.2025
// 🕓 АРХІВАЦІЯ ≥ 23:58 (один раз на добу)
if (timeOnly >= "23:58" && !archiveDoneToday) {
  Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

  bool *param = new bool(true);  // автоматичне архівування з очищенням
  xTaskCreatePinnedToCore(
    saveHistoryTask,   // ❗ фонова задача з підтримкою очищення та видалення старих логів
    "SaveHistory",
    8192,
    param,
    1,
    NULL,
    1  // ядро 1 (можна змінити)
  );

  archiveDoneToday = true;
  Serial.println("✅ Архівування розпочато у фоновому режимі.");
}

// 🕛 Створення нового CSV ≥ 00:01
if (timeOnly >= "00:01" && !fileCreated && today != lastInitDate) {
  String filename = "/log/" + today + ".csv";

  if (!SPIFFS.exists(filename)) {
    File f = SPIFFS.open(filename, FILE_WRITE);
    if (f) {
      f.write(0xEF); f.write(0xBB); f.write(0xBF);  // BOM для UTF-8
      f.println("Time,Temperature,Humidity,Pressure");
      f.close();
      Serial.println("📁 Створено новий архівний файл: " + filename);
    } else {
      Serial.println("❌ Помилка створення архівного файлу");
    }
  } else {
    Serial.println("ℹ️ Файл архіву вже існує: " + filename);
  }

  fileCreated = true;
  lastInitDate = today;
}

/*
  // 🕓 АРХІВАЦІЯ ≥ 23:58 (один раз на добу)
  if (timeOnly >= "23:58" && !archiveDoneToday) {
    Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

    bool *param = new bool(true);  // автоматичне з очищенням
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveDoneToday = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 Створення нового CSV >= 00:01
  if (timeOnly >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";
    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Створено новий архівний файл: " + filename);
      } else {
        Serial.println("❌ Помилка створення архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл архіву вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }
  */
/*
  // ♻️ Скидання флагів після 01:00. Перенесено зі змінами на початок архівації 
  if (currentTime >= "01:00") {
    if (archiveDoneToday || fileCreated) {
      Serial.println("♻️ Час >= 01:00 — скидаю флаги архівації/створення");
    }
    archiveDoneToday = false;
    fileCreated = false;
  }
  */
/* // не спрацювало автоматичне архівування 12_07_2025
    static String lastArchiveDate = "";  // дата останнього архіву
  static String lastInitDate = "";       // дата останнього створеного файлу
  static bool archiveStarted = false;    // 🔁 Щоб не запускати архів кілька разів
  static bool fileCreated = false;       // 🔁 Щоб файл створився лише 1 раз 
  String currentTime = getCurrentTime(); // "HH:MM"
  String today = getTodayDate();         // "YYYY-MM-DD"

  // 🕓 Автоматичне архівування >= 23:58
  if (currentTime >= "23:58" && !archiveStarted && lastArchiveDate != today) {
    Serial.println("🕓 Поточний час >= 23:58 — починаю архівацію...");

    bool *param = new bool(true);  // true = очищення + видалення старих
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveStarted = true;
    lastArchiveDate = today;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 СТВОРЕННЯ АРХІВНОГО ФАЙЛУ НА НОВУ ДОБУ (00:01+)
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Новий архівний файл створено: " + filename);
      } else {
        Serial.println("❌ Помилка створення архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл архіву вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  // 🧹 Скидання флагів лише якщо дата ЗМІНИЛАСЯ
  if (today != lastArchiveDate) {
    if (archiveStarted || fileCreated) {
      Serial.println("♻️ Нова дата — скидаю флаги архівації та створення.");
    }
    archiveStarted = false;
    fileCreated = false;
  }

  // ... решта логіки: сенсори, дисплей, сервер тощо
*/
 /* //не перевірений варіант. Здається має проблему із скиданням флага архівування 11.07.2025
  static bool archiveStarted = false;  // 🔁 Щоб не запускати архів кілька разів
  static bool fileCreated = false;     // 🔁 Щоб файл створився лише 1 раз
  static String lastInitDate = "";     // 🕛 Для уникнення повторного створення

  String currentTime = getCurrentTime();   // Формат "HH:MM"
  String today = getTodayDate();           // Формат "YYYY-MM-DD"

  // ================================================================
  // 🕓 НАДІЙНЕ АВТОМАТИЧНЕ АРХІВУВАННЯ О 23:58
  // ================================================================
  if (currentTime >= "23:58" && !archiveStarted) {
    Serial.println("🕓 Поточний час >= 23:58 — починаю архівацію поточних даних...");

    bool *param = new bool(true);  // true → після збереження очищуємо масив
    xTaskCreatePinnedToCore(
      saveHistoryTask,
      "SaveHistory",
      8192,
      param,
      1,
      NULL,
      1
    );

    archiveStarted = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // ================================================================
  // 🕛 СТВОРЕННЯ АРХІВНОГО ФАЙЛУ НА НОВУ ДОБУ (00:01+)
  // ================================================================
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);  // UTF-8 BOM
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Створено новий файл архіву: " + filename);
      } else {
        Serial.println("❌ Помилка створення нового архівного файлу!");
      }
    } else {
      Serial.println("ℹ️ Файл архіву на сьогодні вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  //================++++++++++++++++++++++++

 
  // ================================================================
  // 🔄 СКИДАННЯ ФЛАГІВ ПІСЛЯ 01:00
  // ================================================================ 
  if (currentTime >= "01:00") {
    if (archiveStarted || fileCreated) {
      Serial.println("♻️ Час > 01:00 — скидаю флаги архівації/створення файлу.");
    }
    archiveStarted = false;
    fileCreated = false;
  }
 */
  // 🔽 Далі розміщується інша логіка: сенсори, дисплей, веб-сервер тощо...


/*
   // 🔧 ДОДАТКОВИЙ блок контролю часу, вставити НА САМОМУ ПОЧАТКУ
  static String lastSavedDate = "";
  static String lastInitDate = "";

  String currentTime = getCurrentTime();  // формат "HH:MM"
  String today = getTodayDate();

  if (currentTime == "23:59" && today != lastSavedDate) {
    Serial.println("🕓 23:59 — Архівую поточні дані у " + today);
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, NULL, 1, NULL, 1);
    lastSavedDate = today;
  }

  if (currentTime == "00:01" && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";
    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Новий архівний файл створено на день: " + today);
      } else {
        Serial.println("❌ Помилка створення нового архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл на " + today + " вже існує.");
    }
    lastInitDate = today;
  }
*/
  // 🔽 Далі залишається ВАША основна логіка loop():
  // - опитування сенсорів
  // - оновлення дисплея
  // - обробка веб-запитів
  // - оновлення графіків і т.д


  // ==========================================================
  // Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ
  // Цей блок перевіряє, чи минула 5 хвилин (300000 мс) з останнього оновлення даних.
  // Він працює незалежно від того, що відображається на екрані.
  // ==========================================================
// Періодичне зчитування BME280 з інтервалом 60 секунд і контролем коректності даних
static uint32_t lastUpdate = 0;
//uint32_t currentMillis = millis();

if (currentMillis - lastUpdate > 60000) {
  lastUpdate = currentMillis; 

  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float press = bme.readPressure();

  // Перевірка на адекватність даних
  if (temp < -40 || temp > 85) {
    Serial.printf("⚠️ Температура поза межами: %.1f°C — запис пропущено\n", temp);
    return;
  }
  if (hum < 0 || hum > 100) {
    Serial.printf("⚠️ Вологість поза межами: %.1f%% — запис пропущено\n", hum);
    return;
  }
  if (press < 80000 || press > 110000) {
    Serial.printf("⚠️ Тиск поза межами: %.2f Па — запис пропущено\n", press);
    return;
  }

  // Збереження у глобальні змінні (якщо потрібно)
  lastTemp = temp;
  lastHum = hum;
  lastPress = press;

  // Поточний час у форматі рядка
  String timeStr = getCurrentTime();

  // Збереження у історію
  storeToHistory(lastTemp, lastHum, lastPress, timeStr);

  Serial.println("✅ Дані BME280 оновлено і збережено");

  // Діагностика памʼяті
    Serial.printf("🧠 Вільно Heap: %u байт\n", ESP.getFreeHeap());
}


  /*
  //не перевіряє коректність даних, але працює
  static uint32_t lastUpdate = 0;
  if (currentMillis - lastUpdate > 60000) {
    lastUpdate = currentMillis;
    lastTemp = bme.readTemperature();
    lastHum = bme.readHumidity();
    lastPress = bme.readPressure();

    String timeStr = getCurrentTime();
    storeToHistory(lastTemp, lastHum, lastPress, timeStr);

    Serial.println("Дані ВМЕ280 оновлено і збережено");
  }
  */
// У вашій функції loop() знайдіть цей блок:
// ==========================================================
// Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
// ==========================================================
/*
  //static String lastSavedDate;
  //String today = getTodayDate();
  if (today != lastSavedDate) {
    Serial.println("Зміна дати виявлена! Запускаю архівування та очищення."); // Для налагодження

    // ЗАМІНІТЬ ЦЕЙ РЯДОК:
    // performHistoryFileSave();

    // НА ЦЕЙ БЛОК:
    xTaskCreate(
      saveHistoryTask,       // Функція, що буде виконуватись як завдання
      "SaveHistory",         // Назва завдання (для дебагу)
      8192,                  // Розмір стека в байтах
      NULL,                  // Параметр, що передається в завдання (тут не потрібен)
      1,                     // Пріоритет завдання (0 - найнижчий. 1 - вже робочий)
      NULL                   // Дескриптор завдання (якщо потрібно його контролювати)
    );
//Test
    clearOldLogs(3);        // Очистити старі файли логів (3 дні)
    clearHistoryData();      // Очистити масив історії для нового дня
    lastSavedDate = today;   // Оновити останню збережену дату
  }
*/
/*
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
*/
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
          //++++++++++++++++++++++++++++++++++++++

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
// End loop()
//++++++++++++++++++++++++++++++++++++++
// ==== Нова функція: Основний функціонал збереження історії у файл ====
// Ця функція містить код, який безпосередньо записує дані в SPIFFS.
// Вона не приймає аргументів і не має нескінченних циклів чи затримок,
// тому її можна безпечно викликати з void loop().

void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file = SPIFFS.open(filename, FILE_APPEND);

  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл для архівації!");
    return;
  }

  // Записуємо лише нові, ще не збережені дані
  Serial.printf("Архівація з рядка: %u\n", lastSavedIndex);
    //Serial.print("Архівація з рядка: "); // <--- Додайте цей рядок
    //Serial.println(lastSavedIndex); // <--- Додайте цей рядок
  for (int i = lastSavedIndex; i < historyIndex; ++i) {
    file.printf("%s,%.2f,%.2f,%.2f\n",
                history[i].timeStr.c_str(),
                history[i].temperature,
                history[i].humidity,
                history[i].pressure);
  }

  file.close();

  Serial.printf("📦 Архівовано %d нових записів у %s\n", historyIndex - lastSavedIndex, filename.c_str());

  // Оновлюємо межу архівації
  lastSavedIndex = historyIndex;
Serial.printf("Оновлене значення lastSavedIndex: %u\n", lastSavedIndex);
    //Serial.print("Оновлене значення lastArchivedIndex: "); // <--- Додайте цей рядок
    //Serial.println(lastSavedIndex); // <--- Додайте цей рядок
}

/*
//Працюючий варіант. Є підозра, що додає зайві пробіли при формуванні даних в масив
//Мій варіант збереження історії 18.07.2025
//int lastSavedIndex = 0;  // глобальна змінна, ініціалізована один раз у верху
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file = SPIFFS.open(filename, FILE_APPEND);

  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл для архівації!");
    return;
  }

  // Записуємо лише нові, ще не збережені дані
  Serial.printf("Архівація з рядка: %u\n", lastSavedIndex);
    //Serial.print("Архівація з рядка: "); // <--- Додайте цей рядок
    //Serial.println(lastSavedIndex); // <--- Додайте цей рядок
  for (int i = lastSavedIndex; i < historyIndex; ++i) {
    file.printf("%s,%.2f,%.2f,%.2f\n",
                history[i].timeStr.c_str(),
                history[i].temperature,
                history[i].humidity,
                history[i].pressure);
  }

  file.close();

  Serial.printf("📦 Архівовано %d нових записів у %s\n", historyIndex - lastSavedIndex, filename.c_str());

  // Оновлюємо межу архівації
  lastSavedIndex = historyIndex;
Serial.printf("Оновлене значення lastSavedIndex: %u\n", lastSavedIndex);
    //Serial.print("Оновлене значення lastArchivedIndex: "); // <--- Додайте цей рядок
    //Serial.println(lastSavedIndex); // <--- Додайте цей рядок
}
*/
/*
String lastSavedTimestamp = "";  // глобальна змінна — запам'ятовуємо останній збережений час

void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити архів для дозапису");
    return;
  }

  int newRecords = 0;

  for (int i = lastArchivedIndex; i < historyIndex; ++i) {
    const BMEData& entry = history[i];

    if (entry.timeStr != "" && entry.timeStr != lastSavedTimestamp) {
      file.printf("%s,%.2f,%.2f,%.2f\n",
        entry.timeStr.c_str(), entry.temperature, entry.humidity, entry.pressure);
      
      lastSavedTimestamp = entry.timeStr;  // запам’ятовуємо останній записаний час
      ++newRecords;
    }
  }

  lastArchivedIndex = historyIndex;
  file.close();

  Serial.printf("📁 Збережено %d нових записів у %s\n", newRecords, filename.c_str());
}
*/


/*
//Мій варіант збереження історії 18.07.2025
//int lastSavedIndex = 0;  // глобальна змінна, ініціалізована один раз
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для архівації.");
    return;
  }

  int saved = 0;

  for (int i = lastArchivedIndex; i < historyIndex; ++i) {
    if (history[i].timeStr != "") {
      file.printf("%s,%.2f,%.2f,%.2f\n",
                  history[i].timeStr.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      saved++;
    }
  }

  file.close();
  lastArchivedIndex = historyIndex;  // оновлюємо межу збереження
  Serial.printf("✅ У файл %s збережено %d нових записів\n", filename.c_str(), saved);
}
*/

/*
//Мій варіант збереження історії 18.07.2025
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file;

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    SPIFFS.mkdir("/log");
  }

  bool fileExists = SPIFFS.exists(filename);
  file = SPIFFS.open(filename, FILE_APPEND);

  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису архіву!");
    return;
  }

  if (!fileExists) {
    file.println("Time,Temperature,Humidity,Pressure");
  }

  Serial.println("🧪 Запис у файл архіву...");
  int recordsWritten = 0;

  for (int i = lastArchivedIndex; i < historyIndex; i++) {
    BMEData entry = history[i];
    String line = entry.timeStr + "," + String(entry.temperature, 2) + "," + String(entry.humidity, 2) + "," + String(entry.pressure, 2);
    file.println(line);
    recordsWritten++;
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), recordsWritten);

  lastArchivedIndex = historyIndex;
}
*/
/*
// === 🔁 СКИДАННЯ ІНДЕКСІВ НА ПОЧАТКУ ДОБИ ===
void resetHistoryForNewDay() {
  historyIndex = 0;
  lastArchivedIndex = 0;
}
*/
/*
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    SPIFFS.mkdir("/log");
  }

  std::set<String> existingTimestamps;

  if (SPIFFS.exists(filename)) {
    File existingFile = SPIFFS.open(filename, "r");
    while (existingFile.available()) {
      String line = existingFile.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      if (commaIndex > 0) {
        String timestamp = line.substring(0, commaIndex);
        existingTimestamps.insert(timestamp);
      }
    }
    existingFile.close();
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Помилка відкриття файлу для запису");
    return;
  }

  int savedCount = 0;
  for (int i = 0; i < historyIndex; ++i) {
    String timestamp = history[i].timestamp;
    if (existingTimestamps.count(timestamp) == 0) {
      file.printf("%s,%.2f,%.2f,%.2f\n", timestamp.c_str(), history[i].temperature, history[i].humidity, history[i].pressure);
      savedCount++;
    }
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), savedCount);

  if (historyIndex > 0) {
    historyIndex = 0;
  }
}
*/
/*
void performHistoryFileSave() {
  const char* filename = "/history.csv";
  String lastTimestampStr = "";
  time_t lastTimestamp = 0;

  // --- 1. Читаємо останній timestamp з історії
  File existing = SPIFFS.open(filename, FILE_READ);
  if (existing) {
    while (existing.available()) {
      String line = existing.readStringUntil('\n');
      line.trim();  // очистка пробілів

      if (line.length() > 10 && line.indexOf(',') != -1) {
        lastTimestampStr = line.substring(0, line.indexOf(','));
      }
    }
    existing.close();
    Serial.println("🔍 Останній час у файлі: " + lastTimestampStr);

    struct tm tm = {};
    if (strptime(lastTimestampStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      lastTimestamp = mktime(&tm);
      Serial.printf("📅 Розпізнано останній час: %ld (UNIX)\n", lastTimestamp);
    } else {
      Serial.println("❌ strptime() не зміг розпізнати час.");
    }
  } else {
    Serial.println("⚠️ Файл історії не існує або не відкрито.");
  }

  // --- 2. Записуємо нові записи з history[]
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для дозапису.");
    return;
  }

  int written = 0, skipped = 0;
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    String ts = history[i].timeStr;
    ts.trim();

    if (ts == "") continue;

    struct tm tm = {};
    if (!strptime(ts.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      Serial.println("❗ Пропущено некоректний час: " + ts);
      continue;
    }

    time_t entryTime = mktime(&tm);
    if (entryTime > lastTimestamp) {
      file.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      Serial.println("✅ Додано: " + ts);
      written++;
    } else {
      Serial.println("⏩ Пропущено: " + ts);
      skipped++;
    }
  }

  file.close();
  if (historyCount > 0) {
    lastArchivedTimestamp = history[historyCount - 1].timestamp;
  }

  Serial.printf("📄 Збережено: %d, Пропущено: %d\n", written, skipped);
}
*/

/*
//======================================
//оновлений фрагмент функції performHistoryFileSave() з урахуванням фільтрації на дублікати
//Дублювання залишилося
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  Serial.printf("➡️  Файл: %s\n", filename.c_str());

  // 1. Прочитаємо наявні рядки в файл архіву
  std::set<String> existingTimestamps;

  if (SPIFFS.exists(filename)) {
    File existingFile = SPIFFS.open(filename, FILE_READ);
    if (existingFile) {
      while (existingFile.available()) {
        String line = existingFile.readStringUntil('\n');
        int commaIndex = line.indexOf(',');
        if (commaIndex > 0) {
          String ts = line.substring(0, commaIndex);
          ts.trim();
          existingTimestamps.insert(ts);
        }
      }
      existingFile.close();
    }
  }

  // 2. Відкриваємо файл на дозапис
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл історії для запису!");
    return;
  }

  int added = 0;
  for (int i = 0; i < historyIndex; ++i) {
    String ts = history[i].timeStr;
    if (existingTimestamps.find(ts) == existingTimestamps.end()) {
      file.printf("%s,%.2f,%.2f,%.2f\n",
                  ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      ++added;
    } else {
      Serial.printf("⚠️ Пропущено дубль: %s\n", ts.c_str());
    }
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d нових записів)\n", filename.c_str(), added);

  // 3. Очистимо буфер
  historyIndex = 0;
}
*/
/*
//Файл відкривається один раз, перевіряється, чи дані вже є у файлі (уникаємо дублів).
//Після збереження — очищення historyIndex = 0.
//17.07.2025 Файли дублюються!
void performHistoryFileSave() {
  File file = SPIFFS.open("/history.csv", FILE_APPEND);
  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл історії для дозапису!");
    return;
  }

  for (int i = 0; i < historyIndex; ++i) {
    if (history[i].timeStr != "") {
      file.printf("%s,%.2f,%.2f,%.2f\n",
                  history[i].timeStr.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
    }
  }

  file.close();
  historyIndex = 0;  // очищення після запису
  Serial.println("✅ Дані історії збережено у файл і буфер очищено.");
}

*/
/*
// не перевірялася. 17.07.2025
 void performHistoryFileSave() {
  const char* filename = "/history.csv";
  String lastTimestampStr = "";
  time_t lastTimestamp = 0;

  // --- 1. Читаємо останній timestamp з історії
  File existing = SPIFFS.open(filename, FILE_READ);
  if (existing) {
    while (existing.available()) {
      String line = existing.readStringUntil('\n');
      line.trim();  // очистка пробілів

      if (line.length() > 10 && line.indexOf(',') != -1) {
        lastTimestampStr = line.substring(0, line.indexOf(','));
      }
    }
    existing.close();
    Serial.println("🔍 Останній час у файлі: " + lastTimestampStr);

    struct tm tm = {};
    if (strptime(lastTimestampStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      lastTimestamp = mktime(&tm);
      Serial.printf("📅 Розпізнано останній час: %ld (UNIX)\n", lastTimestamp);
    } else {
      Serial.println("❌ strptime() не зміг розпізнати час.");
    }
  } else {
    Serial.println("⚠️ Файл історії не існує або не відкрито.");
  }

  // --- 2. Записуємо нові записи з history[]
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для дозапису.");
    return;
  }

  int written = 0, skipped = 0;
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    String ts = history[i].timeStr;
    ts.trim();

    if (ts == "") continue;

    struct tm tm = {};
    if (!strptime(ts.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      Serial.println("❗ Пропущено некоректний час: " + ts);
      continue;
    }

    time_t entryTime = mktime(&tm);
    if (entryTime > lastTimestamp) {
      file.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      Serial.println("✅ Додано: " + ts);
      written++;
    } else {
      Serial.println("⏩ Пропущено: " + ts);
      skipped++;
    }
  }

  file.close();
  Serial.printf("📄 Збережено: %d, Пропущено: %d\n", written, skipped);
}
*/

/*
 // Працює,але при дозапису дублює дані
void performHistoryFileSave() {
  const char* filename = "/history.csv";
  String lastTimestampStr = "";
  time_t lastTimestamp = 0;

  // 1. Читаємо останній рядок
  File existing = SPIFFS.open(filename);
  if (existing) {
    while (existing.available()) {
      String line = existing.readStringUntil('\n');
      if (line.length() > 10) {
        line.trim();  // 🔴 Видаляємо пробіли на початку/кінці
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1) {
          lastTimestampStr = line.substring(0, commaIndex);
        }
      }
    }
    existing.close();

    struct tm tm = {};
    if (strptime(lastTimestampStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      lastTimestamp = mktime(&tm);
    }
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл.");
    return;
  }

  int written = 0, skipped = 0;
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    String ts = history[i].timeStr;
    ts.trim();  // 🔴 очищення перед парсингом

    if (ts == "") continue;

    struct tm tm = {};
    if (!strptime(ts.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      Serial.println("⛔ Некоректний timestamp: " + ts);
      continue;
    }

    time_t entryTime = mktime(&tm);
    if (entryTime > lastTimestamp) {
      // Формуємо нормалізований рядок (без пробілів після коми)
      file.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      written++;
    } else {
      skipped++;
      Serial.println("⏩ Пропущено дубль: " + ts);
    }
  }

  file.close();
  Serial.printf("✅ Збережено: %d, Пропущено: %d\n", written, skipped);
}
*/
 /*
 // Працює,але при дозапису дублює дані
void performHistoryFileSave() {
  const char* filename = "/history.csv";
  String lastTimestampStr = "";
  time_t lastTimestamp = 0;

  // 1. Читаємо останній рядок з архіву
  File existing = SPIFFS.open(filename);
  if (existing) {
    while (existing.available()) {
      String line = existing.readStringUntil('\n');
      if (line.length() > 10) {
        lastTimestampStr = line.substring(0, line.indexOf(','));  // YYYY-MM-DD HH:MM:SS
      }
    }
    existing.close();
    Serial.println("📅 Останній запис у файлі: " + lastTimestampStr);

    struct tm tm = {};
    if (strptime(lastTimestampStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      lastTimestamp = mktime(&tm);
    } else {
      Serial.println("⚠️ Не вдалося розпарсити останній timestamp.");
    }
  } else {
    Serial.println("ℹ️ Архів не існує, створюємо новий.");
  }

  // 2. Відкриваємо для дозапису
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Помилка відкриття файлу для дозапису.");
    return;
  }

  int written = 0, skipped = 0;
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    String ts = history[i].timeStr;
    if (ts == "") continue;

    struct tm tm = {};
    if (!strptime(ts.c_str(), "%Y-%m-%d %H:%M:%S", &tm)) {
      Serial.println("⛔ Невірний формат часу: " + ts);
      continue;
    }

    time_t entryTime = mktime(&tm);
    if (entryTime > lastTimestamp) {
      file.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      written++;
    } else {
      skipped++;
      Serial.println("⏩ Пропущено старий запис: " + ts);
    }
  }

  file.close();
  Serial.printf("✅ Записано %d нових, пропущено %d.\n", written, skipped);
}
*/
/*
// Працює,але при дозапису дублює дані
void performHistoryFileSave() {
  const char* filename = "/history.csv";

  // Отримуємо останню дату з існуючого файлу
  String lastTimestamp = "";
  {
    File existing = SPIFFS.open(filename);
    if (existing) {
      while (existing.available()) {
        String line = existing.readStringUntil('\n');
        if (line.length() > 10) lastTimestamp = line.substring(0, line.indexOf(','));
      }
      existing.close();
      Serial.println("📅 Останній запис у архіві: " + lastTimestamp);
    } else {
      Serial.println("ℹ️ Файл історії ще не існує, створюємо новий.");
    }
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл історії для дозапису!");
    return;
  }

  int written = 0, skipped = 0;
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    String ts = history[i].timeStr;
    if (ts == "") continue;

    if (ts > lastTimestamp) {
      file.printf("%s,%.2f,%.2f,%.2f\n", ts.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
      written++;
    } else {
      skipped++;
      Serial.println("⏩ Пропущено (старий запис): " + ts);
    }
  }

  file.close();
  Serial.printf("✅ Записано: %d нових записів. Пропущено: %d.\n", written, skipped);
}
*/
/*
// Працює,але при дозапису дублює дані
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
*/
//++++++++++++++++++++++++++++++++++++++
void executeDisplayMode(DisplayMode mode) {
    tft.fillScreen(TFT_BLACK); 

    switch (mode) {
        case MODE_NONE:
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("Режим неактивний", screenW / 2, screenH / 2);
            break;
        case MODE_TOLYA_VITA:
            displayTolyaVita(); // <--- ДОДАЙТЕ ЦЕЙ РЯДОК!
            break;
        case MODE_UKRAINIAN:
            displayUkrainian(); // Якщо ця функція також не викликається, додайте і її
            break;
        case MODE_VIVAT:
            vivat(); 
            break;
        case MODE_SLAVA_UKRAINI:
            slava_ukraini();
            break;
        case MODE_DATETIME:
            displayDateTime();
            break;
        case MODE_SHEVCHENKO:
            shevchenko();
            break;
        case MODE_BME_DATA:
            displayBMEData();
            break;
        default:
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("Невідомий режим", screenW / 2, screenH / 2);
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++
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

// === 🔁 СКИДАННЯ ІНДЕКСІВ НА ПОЧАТКУ ДОБИ ===
//Оновлено 19.07.2025
void resetHistoryForNewDay() {
  clearHistoryData();   // повне очищення масиву
  historyIndex = 0;     // лічильник записів
  lastSavedIndex = 0;   // межа архівації
  Serial.println("🔄 Історія очищена: новий день почався");
}

/*
void resetHistoryForNewDay() {
  historyIndex = 0;
  lastArchivedIndex = 0;
  lastSavedIndex = 0;
  Serial.println("🔄 Історія очищена: новий день почався");
}
*/
// === 🌡️ ФУНКЦІЯ ДОДАВАННЯ ЗАПИСУ ДО HISTORY[] ===
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr) {
  if (historyIndex >= maxHistorySize) return;
 
history[historyIndex].timeStr = timeStr;
history[historyIndex].temperature = temp;
history[historyIndex].humidity = hum;
history[historyIndex].pressure = pres;
historyIndex++;

}
