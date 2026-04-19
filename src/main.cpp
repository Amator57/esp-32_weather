// Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ рядок 1088
// Очистити старі файли логів (14 днів)  clearOldLogs(14) - 1073; рядок 417  clearOldLogs(14)
#include <Arduino.h>
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
#include "web_handlers.h"
float tempOffset = 0.0f;  //Початкове значення поправки температури
float humOffset = 0.0f;  // Початкове значення поправки вологості

AsyncWebServer server(80);
Preferences preferences; //Перенесено до web_handlers.h

String deviceName = "ESP32_Weather";
String wifiSSID = "Unknown";
String wifiPassword = ""; // ✅ Оголошуємо змінну для пароля
String tzString = "EET-2EEST,M3.5.0/3,M10.5.0/4";

bool useDST = true;       // Літній час
int tzOffset = 2;  // за замовчуванням для Києва (UTC+2)
// Ініціалізація глобальних змінних для ротації дисплея
bool rotationActive = false;
DisplayMode previousDisplayMode = MODE_NONE; // Ініціалізуємо попередній режим відображення
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
float averpress = 0,  counpress = 0; // Середнє значення тиску та лічильник вимірювань
float avertemperature = 0;
float averhumidity = 0;
int8_t   hour, minute, second;
//Прапорці очищення архіва
volatile bool archivingInProgress = false;
volatile bool clearAfterSaveTriggered = false;
// Флаги для кожного слоту
bool f_0 = true, f_4 = true, f_8 = true, f_12 = true, f_16 = true, f_20 = true;
bool arhiv = false;
//Оновлення даних для архіву та відображення

unsigned long lastDisplayUpdate = 0;
unsigned long lastArchiveUpdate = 0;

const unsigned long DISPLAY_INTERVAL = 30000;   // 30 секунд
const unsigned long ARCHIVE_INTERVAL = 300000;  // 5 хвилин


long x;//усереднення тиску
// === 🔧 ЗМІННІ ДЛЯ ІНДЕКСУВАННЯ АРХІВАЦІЇ ===
int lastArchivedIndex = 0;
unsigned char flag_s=1;
unsigned char flag_w=1;
unsigned char  ind;
bool testArchiveFlag = true; //Флаг тестового архівування
#define MAX_POINTS 1440
#define SENSOR_POWER_PIN   19     // Заміни на свій пін живлення
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
void performHistoryFileSave();// Оголошення нової функції для збереження історії
void resetHistoryForNewDay();
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr);
void clearHistoryData();     // Оголошення для функції очищення масиву поточних даних
bool checkAndRecoverBME280(); // Оголошення для функції перевірки та відновлення BME280
bool isBME280DataValid(float temp, float hum, float press);
bool restartBME280();


//void readBME280Data();
// Допоміжні функції для відображення на TFT
String getTodayDate(int offset);
void displayTolyaVita() {
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
  vivat(); // Ваша функція для відображення "Vivat"
}

void displaySlavaUkraini() {
  tft.fillScreen(TFT_BLACK);
  slava_ukraini(); // Ваша функція для відображення "Slava Ukraini"
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
  shevchenko(); // Ваша функція для відображення Шевченка
}

void displayBMEData() {
  
  tft.fillScreen(TFT_BLACK); // Очищаємо екран перед відображенням даних BME
  tft.loadFont(My_ariali_24);
   //temperature = lastTemp;
    float avgTemperature = lastTemp;
    float avgPressure = lastPress;
    float avgHumidity = lastHum;
    //humidity = lastHum;
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
  //tft.print(temperature); // Глобальна змінна 'temperature'
  tft.print(avgTemperature); // Глобальна змінна 'temperature'
  tft.drawString(" C", 250, 110);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Вологість:", 20, 170);
  tft.setCursor(155, 170);
  //tft.print(humidity); // Глобальна змінна 'humidity'
  tft.print(avgHumidity); // Глобальна змінна 'humidity'
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
  if (!getLocalTime(&timeinfo, 10)) {
    DateTime now = rtc.now();
    char buf[11];
    sprintf(buf, "%04d-%02d-%02d", now.year(), now.month(), now.day());
    return String(buf);
  }
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}

// --- Декларації наперед (збережені з оригіналу) ---
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

//////////////
// ОНОВЛЕНО: Формат дати/часу на YYYY-MM-DD HH:MM:SS для кращої сумісності
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    DateTime now = rtc.now();
    char buf[20];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
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
  if (!getLocalTime(&timeinfo, 10)) {
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
  if (var == "WIFI_PASSWORD") return wifiPassword;
  if (var == "TZ_OFFSET") return String(tzOffset);  // нова змінна: часовий зсув UTC, напр. -2
  if (var == "USE_DST_CHECKED") return useDST ? "checked" : "";
  if (var == "TEMP") return String(bme.readTemperature()+ tempOffset, 1);
  if (var == "HUM") return String(bme.readHumidity()+ humOffset, 0);
  if (var == "PRES") return String(bme.readPressure() / 100.0F, 0);
  if (var == "SW1_CHECKED") return switch1 ? "checked" : "";
  if (var == "SW2_CHECKED") return switch2 ? "checked" : "";
  if (var == "SW3_CHECKED") return switch3 ? "checked" : "";
  if (var == "SW4_CHECKED") return switch4 ? "checked" : "";
  if (var == "SW5_CHECKED") return switch5 ? "checked" : "";
  if (var == "SW6_CHECKED") return switch6 ? "checked" : "";
  if (var == "SW7_CHECKED") return switch7 ? "checked" : "";
  if (var == "TEMP_OFFSET") {
    return String(tempOffset, 1); // округлено до 1 знаку після коми
}
  if (var == "HUM_OFFSET") {
    return String(humOffset, 1);} // округлено до 1 знаку після коми
  return String();
}

//+++++++++++++++++++++++++++++++++
void saveConfig() {
  preferences.begin("config", false);
  preferences.putString("deviceName", deviceName);
  preferences.putString("wifiSSID", wifiSSID);
  preferences.putString("wifiPassword", wifiPassword);
  preferences.putBool("use_dst_b", useDST);
  preferences.putFloat("tempOffset", tempOffset);   // ✅ збереження поправки температури
  preferences.putFloat("humOffset", humOffset);     // ✅ збереження поправки вологості
  preferences.putBool("switch1", switch1);
  preferences.putBool("switch2", switch2);
  preferences.putBool("switch3", switch3);
  preferences.putBool("switch4", switch4);
  preferences.putBool("switch5", switch5);
  preferences.putBool("switch6", switch6);
  preferences.putBool("switch7", switch7);
  preferences.putInt("tz_off", tzOffset);
  preferences.end();
}

//+++++++++++++++++++++++++++++++++
/*
//Перенесено до web_handlers.h
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
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// --- Отримати сьогоднішню дату у форматі РРРР-ММ-ДД ---
/* Закоментовано 03.08.2025, замінено на getTodayDate() з параметром offset
String getTodayDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "unknown";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}
*/
//++++++++++++++++++++++++++++++++++++++
// Замінено 03.08.2025. Архівація та очищення логів кожні 4 години
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate(0) + ".csv";
  Serial.println("📦 Запуск архівації...");
  Serial.printf("➡️ Файл: %s\n", filename.c_str());

  // Переконуємось, що директорія існує
  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл тільки створено (порожній) — додамо заголовок
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  // Записуємо всі записи з history[]
  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (ok) written++;
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);
  lastSavedIndex = historyIndex;

  if (clearAfterSave) {
    clearHistoryData();  // очищаємо history[]
    Serial.println("🧹 Масив history[] очищено після архівації");
  }

  // Очищення старих логів (залишаємо останні 14 днів)
  Serial.println("♻️ Виклик clearOldLogs(14)...");
  clearOldLogs(14);

  vTaskDelete(NULL);
}



//+++++++++++++++++++++++++++++++++
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

//+++++++++++++++++++++++++++++++++

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

//+++++++++++++++++++++++++++++++++
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//+++++++++++++++++++++++++++++++++
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  // === 🖥 ІНІЦІАЛІЗАЦІЯ СЕРІЙНОГО ПОРТУ І TFT ===
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  // === ⏰ RTC, I2C, SPIFFS ===
  Wire.begin();
  rtc.begin();

  Serial.println("Ініціалізація SPIFFS...");
  if (!SPIFFS.begin()) {
    Serial.println("❌ Помилка при монтуванні SPIFFS");
    while (true) { delay(100); }
  }
  Serial.println("✅ SPIFFS успішно змонтовано.");

  resetHistoryForNewDay();
  Serial.println("✅ Індекс історії очищено.");
  //=========================
  //Видалення конкретного лог-файлу
       // deleteLogFile("unknown.csv"); //  deleteLogFile("2025-07-10.csv");
       // Serial.println("✅ Заданий файл історії успішно очищено.");
  //=========================
  if (!SPIFFS.exists("/log")) {
    Serial.println("[SETUP] Директорія /log не знайдена — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити директорію /log!");
      while (true) { delay(100); }
    }
  }

  // === 🌡️ BME280 ===
  digitalWrite(SENSOR_POWER_PIN, HIGH);// Увімкнення TLV70033 - живлення BME 280
    delay(200);
  bme.begin(0x76);
  for (int j = 0; j < MAX_MEASUREMENTS; ++j) {
    history[j].timeStr = "";
    history[j].temperature = 0.0f;
    history[j].humidity = 0.0f;
    history[j].pressure = 0.0f;
  }

  // === 📜 Конфігурація ===
  loadConfig();
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();

  // === 📶 Wi-Fi ===
  Serial.printf("Намагаюся підключитися до SSID: %s\n", wifiSSID.c_str());
  if (wifiSSID != "Unknown" && wifiPassword != "") {
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  } else {
    Serial.println("Немає збережених даних Wi-Fi, пропускаю підключення.");
  }

  delay(3000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ STA не підключився, запускаю AP...");
    if (WiFi.softAP("ESP32_Config", "12345678")) {
      String apIP = WiFi.softAPIP().toString();
      Serial.println("✅ AP запущено. IP: " + apIP);
      tft.drawString("AP Mode: " + apIP, 10, 50);
    }
  } else {
    wifiSSID = WiFi.SSID();
    String ip = WiFi.localIP().toString();
    Serial.println("✅ WiFi: " + wifiSSID + ", IP: " + ip);
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip, 10, 70);
  }

  // === 🕒 Синхронізація NTP → RTC ===
  // Встановлюємо NTP сервера
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Офсет 0, бо ми використовуємо TZ змінну середовища
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("⏳ Синхронізація часу з NTP...");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1000)) { // Чекаємо максимум 1 секунду
      rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
      Serial.println("✅ Час синхронізовано з NTP та збережено в RTC.");
    } else {
      Serial.println("⚠️ Не вдалося отримати час з NTP (таймаут 1с). Використовуємо RTC.");
    }
  } else {
    Serial.println("ℹ️ WiFi не підключено, синхронізація NTP пропущена. Використовуємо RTC.");
  }
Serial.println("♻️ Скидання прапорів — рестарт!");
     f_0 = f_4 = f_8 = f_12 = f_16 = f_20 = true;//Додано 06.08.2025. Скидання прапорів для архівації кожні 4 години
     arhiv = false;//Додано 06.08.2025. Скидання прапорів для архівації кожні 4 години
  // =====================================================================================
  // 🌍 📡 ВЕБ-СЕРВЕР: ГОЛОВНІ HTML-СТОРІНКИ
  // =====================================================================================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
     Serial.println("➡️ [/] Головна сторінка");
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
     Serial.println("➡️ [/settings]");
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });

  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("➡️ [/log_viewer]");
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });
            //Додано мною 27.07.2025. Початок
  // 🔄 Перезавантаження ESP32
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
  
   // 📄 JS для графіка
  server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("➡️ [/graph.js]");
    request->send(SPIFFS, "/graph.js", "application/javascript");
  });

    // 📊 Список логів (HTML)
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
     Serial.printf("🧠 Free Heap before /logs: %u bytes\n", ESP.getFreeHeap());// Додав 30.07.2025
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    response->print(F("<h2>Доступні архіви:</h2><ul>"));
 
    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
      response->print(F("<li>Немає доступних лог-файлів.</li>"));
    } else {
      File file = root.openNextFile();
      while (file) {
        String fullPath = file.name();
        if (fullPath.endsWith(".csv")) {
          String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
          String datePart = fileName;
          datePart.replace(".csv", "");

          response->print("<li>");
          response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
          response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
          response->print("</li>");
        }
        file = root.openNextFile();
      }
    }
    response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
  });

    // 🌡️ BME Дані JSON (поточні)
  server.on("/bme_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    //Serial.printf("🧠 Heap before /bme_data: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("🧠 Вільна пам'ять до /bme_data: %u bytes\n", ESP.getFreeHeap());
    DynamicJsonDocument doc(256);
    doc["temperature"] = bme.readTemperature() + tempOffset;
    doc["humidity"]    = bme.readHumidity() + humOffset;
    doc["pressure"]    = bme.readPressure() / 100.0F;
    doc["presmmhg"]    = bme.readPressure() / 133.322F;
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // === Хендлер збереження налаштувань (/save) === від 04.09.2025
server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request) {

    // --- Отримання параметрів з форми ---
    if (request->hasParam("device_name", true))
        deviceName = request->getParam("device_name", true)->value();

    if (request->hasParam("ssid", true))
        wifiSSID = request->getParam("ssid", true)->value();

    if (request->hasParam("password", true))
        wifiPassword = request->getParam("password", true)->value(); // ДОДАТИ    

    if (request->hasParam("timezone_offset", true))
        tzOffset = request->getParam("timezone_offset", true)->value().toInt();

    useDST = request->hasParam("use_dst", true);

    switch1 = request->hasParam("switch1", true);
    switch2 = request->hasParam("switch2", true);
    switch3 = request->hasParam("switch3", true);
    switch4 = request->hasParam("switch4", true);
    switch5 = request->hasParam("switch5", true);
    switch6 = request->hasParam("switch6", true);
    switch7 = request->hasParam("switch7", true);

    // --- Калібрування температури ---
    if (request->hasParam("tempOffset", true))
        tempOffset = request->getParam("tempOffset", true)->value().toFloat();
    // --- Калібрування вологості ---
    if (request->hasParam("humOffset", true))
        humOffset = request->getParam("humOffset", true)->value().toFloat();
    // --- Збереження всіх параметрів в Preferences ---
    preferences.begin("config", false); // false = запис
        preferences.putString("deviceName", deviceName);
        preferences.putString("wifiSSID", wifiSSID);
        preferences.putString("wifiPassword", wifiPassword); // ДОДАТИ
        preferences.putInt("tz_off", tzOffset);
        preferences.putBool("use_dst_b", useDST);
        preferences.putBool("switch1", switch1);
        preferences.putBool("switch2", switch2);
        preferences.putBool("switch3", switch3);
        preferences.putBool("switch4", switch4);
        preferences.putBool("switch5", switch5);
        preferences.putBool("switch6", switch6);
        preferences.putBool("switch7", switch7);
        preferences.putFloat("tempOffset", tempOffset);
        preferences.putFloat("humOffset", humOffset);
    preferences.end();

    Serial.printf("💾 Калібрування температури збережено. tempOffset = %.1f°C\n", tempOffset);
    Serial.printf("💧 Калібрування вологості збережено: %.1f%%\n", humOffset);
    // --- Встановлення часової зони ---
    String sign = tzOffset >= 0 ? "-" : "+";
    int absOffset = abs(tzOffset);
    String tzString = "UTC" + sign + String(absOffset);
    if (useDST)
        tzString += "DST,M3.5.0/3,M10.5.0/4";
    setenv("TZ", tzString.c_str(), 1);
    tzset();

    // --- Повернення на сторінку налаштувань з повідомленням ---
    request->send(200, "text/html; charset=utf-8",
        "<script>alert('✅ Налаштування збережено!'); location.href='/settings';</script>");
});

/*
    //Збереження налаштувань ОРИГІНАЛ (без tempOffset)
    server.on("/save", HTTP_POST, [&](AsyncWebServerRequest *request){
      if (request->hasParam("device_name", true))
          deviceName = request->getParam("device_name", true)->value();
      if (request->hasParam("ssid", true))
          wifiSSID = request->getParam("ssid", true)->value();

      if (request->hasParam("timezone_offset", true))
          tzOffset = request->getParam("timezone_offset", true)->value().toInt();

      useDST = request->hasParam("use_dst", true);
      // Збереження tempOffset, якщо є
if (request->hasParam("tempOffset", true)) {
    tempOffset = request->getParam("tempOffset", true)->value().toFloat();
    preferences.putFloat("tempOffset", tempOffset);
    Serial.printf("🌡️ Калібрування температури збережено: %.1f°C\n", tempOffset);
}

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
  */
  // 🔄 Ручне збереження історії (без очищення масиву)
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

  // 🔵 3. Головна сторінка (HTML)
  server.on("/main", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/main.html", "text/html", false, processor);
  });

  // =====================================================================================
  // 🌐 **Веб-сервер**: запуск та обробка запитів
  // =====================================================================================
 // =====================================================================================
  // 📊 📈 **JSON API для графіків**
  // =====================================================================================
  // 01.08.2025 Тестування нової функції з обмеженням кількості точок
  // ✅ Динамічна вісь Y: обмеження кількості точок через GET-параметр ?limit=300 (за замовчуванням 300)
  // ✅ Формуємо JSON стрімінгом (без накопичення всього в пам'яті)
  // ✅ Вираховуємо, з якого індекса брати дані (останній блок у межах limit)
  // ✅ Використовуємо AsyncResponseStream для ефективної передачі даних
  // ==== Графік поточних даних (динамічна вісь Y) ====
server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
  //Serial.printf("🧠 Heap before /bme_chart_data: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("🧠 Вільна пам'ять до /bme_chart_data: %d bytes\n", ESP.getFreeHeap());
  // ✅ Обмеження кількості точок через GET-параметр ?limit=300 (за замовчуванням 300)
  int limit = 300;
  if (request->hasParam("limit")) {
    limit = request->getParam("limit")->value().toInt();
    limit = constrain(limit, 50, MAX_POINTS);  // мінімум 50, максимум MAX_POINTS
  }

  // ✅ Формуємо JSON стрімінгом (без накопичення всього в пам'яті)
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print("[");

  bool first = true;

  // ✅ Вираховуємо, з якого індекса брати дані (останній блок у межах limit)
  int startIndex = max(0, historyIndex - limit);

  for (int i = startIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    if (!first) response->print(",");
    first = false;

    response->printf(
      "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
      d.timeStr.c_str(), d.temperature, d.humidity, d.pressure
    );
  }

  response->print("]");
  request->send(response);

  //Serial.printf("✅ /bme_chart_data sent. Heap after: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("✅ /bme_chart_data sent. Вільна пам'ять після: %d bytes\n", ESP.getFreeHeap());
});

  
//////////////////////////////////////////////////
// ==========================
// 📈 JSON для архівних графіків
// ==========================
// ==== Графік архівних даних з файлу CSV ====
//01.08.2025 Тестування нової функції з обмеженням кількості точок
// ✅ Динамічна вісь Y: обмеження кількості точок через GET-параметр ?limit=300 (за замовчуванням 300)
// ✅ Формуємо JSON стрімінгом (без накопичення всього в пам'яті)
// ✅ Вираховуємо, з якого індекса брати дані (останній блок у межах limit)
// ✅ Використовуємо AsyncResponseStream для ефективної передачі даних
// Виклик: /bme_chart_data_archive?date=2025-07-31&limit=300
server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
  Serial.printf("🧠 Heap before /bme_chart_data_archive: %d bytes\n", ESP.getFreeHeap());

  // ✅ 1️⃣ Перевірка параметра дати
  if (!request->hasParam("date")) {
    request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
    return;
  }

  String date = request->getParam("date")->value();
  String path = "/log/" + date + ".csv";

  // ✅ 2️⃣ Перевірка існування файлу
  if (!SPIFFS.exists(path)) {
    request->send(404, "application/json", "[]");  
    Serial.printf("❌ Архів %s не знайдено\n", path.c_str());
    return;
  }

  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    request->send(500, "application/json", "[]");
    Serial.printf("❌ Помилка відкриття файлу %s\n", path.c_str());
    return;
  }

  // ✅ 3️⃣ Читаємо limit (за замовчуванням 300)
  int limit = 300;
  if (request->hasParam("limit")) {
    limit = request->getParam("limit")->value().toInt();
    limit = constrain(limit, 50, 2000);  // безпечний діапазон
  }

  // ✅ 4️⃣ Підрахунок кількості рядків у файлі (для визначення з якого місця читати)
  int totalLines = 0;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0 && !line.startsWith("Time")) {
      totalLines++;
    }
  }
  file.close();

  int startLine = max(0, totalLines - limit);  

  // ✅ 5️⃣ Відкриваємо файл вдруге, щоб стрімити JSON
  file = SPIFFS.open(path, FILE_READ);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print("[");

  bool first = true;
  int currentLine = 0;

  // ✅ 6️⃣ Стрімінг даних з файлу
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line == "" || line.startsWith("Time")) continue;

    if (currentLine++ < startLine) continue; // пропускаємо старі рядки

    // Парсимо CSV
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

  file.close();
 // Serial.printf("✅ Архів %s відправлено (останні %d рядків). Heap after: %d bytes\n",
 //                path.c_str(), limit, ESP.getFreeHeap());
  Serial.printf("✅ Архів %s відправлено (останні %d рядків). Вільна пам'ять після: %d bytes\n",
                path.c_str(), limit, ESP.getFreeHeap());
});


  // 🔵 3. JSON для зовнішніх запитів (динамічний масштаб)
  server.on("/api/history.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/log/2025-07-14.csv");  // тестова дата
    if (!file || file.isDirectory()) {
      request->send(404, "application/json", "{\"error\":\"Файл не знайдено\"}");
      return;
    }

    DynamicJsonDocument doc(8192);
    JsonArray arr = doc.to<JsonArray>();
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;

      int t1 = line.indexOf(',');
      int t2 = line.indexOf(',', t1 + 1);
      int t3 = line.indexOf(',', t2 + 1);
      if (t1 > 0 && t2 > t1 && t3 > t2) {
        String timeVal = line.substring(0, t1);
        float temp = line.substring(t1 + 1, t2).toFloat();
        float hum  = line.substring(t2 + 1, t3).toFloat();
        float pres = line.substring(t3 + 1).toFloat();

        JsonObject obj = arr.createNestedObject();
        obj["time"] = timeVal;
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

  // =====================================================================================
  // 🗂 ОБСЛУГОВУВАННЯ СТАТИКИ, ЛОГІВ ТА 404
  // =====================================================================================
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/log/", SPIFFS, "/log/").setCacheControl("no-store");

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
  });

  // =====================================================================================
  // 🚀 ЗАПУСК СЕРВЕРА
  // =====================================================================================
  server.begin();
  Serial.println("🌐 Веб-сервер запущено на порту 80");
  // === Перший екран на TFT ===
  currentDisplayMode = MODE_DATETIME;
  executeDisplayMode(currentDisplayMode);
  displayModeStartTime = millis();
}


void loop() {
  unsigned long currentMillis = millis();
  static String lastDateReset = "";
  static bool archiveDoneToday = false;
  static bool fileCreated = false;
  static String lastInitDate = ""; 
/*
• 	static у функції або глобальному контексті означає, що змінна зберігає своє значення між викликами, 
    але видима лише в межах того файлу або функції, де оголошена.
• 	lastInitDate — зберігає дату, коли востаннє було створено архівний файл.
• 	static гарантує, що значення не буде втрачено між циклами  або викликами функції, 
    якщо вона оголошена локально.
• 	Ініціалізація = "" означає, що на старті значення порожнє, тобто архів ще не створено.
*/
  String timeOnly = getShortTime();  // HH:MM
  String currentTime = getCurrentTime();   // "YYYY-MM-DD-HH:MM:SS" 
  String today = getTodayDate(0);     // YYYY-MM-DD
 // static String lastArchiveSlot = "";   // відстежуємо останню 4-годинну архівацію. Закоментовано 06.08.2025

  //String today = getTodayDate();      // ✅ рядок потрібен! Оновлює актуальну дату
  // Заміна 03.08.2025. Архівація та очищення логів кожні 4 години
  // ✅ 1️⃣ Скидання прапорів на початку доби
  if (timeOnly >= "00:01" && today != lastDateReset) {
    Serial.println("♻️ Скидання прапорів — нова дата!");
     //f_0 = f_4 = f_8 = f_12 = f_16 = f_20 = true;//Додано 06.08.2025. Скидання прапорів для архівації кожні 4 години
     //arhiv = false;//Додано 06.08.2025. Скидання прапорів для архівації кожні 4 години
    fileCreated = false;
   // lastArchiveSlot = "";  // скидаємо слот, щоб о 00:00 теж спрацювала архівація. Закоментовано 06.08.2025
    lastDateReset = today;
    resetHistoryForNewDay();
  }
/*
потребує перевірки
  if (today != lastInitDate) {
    lastInitDate = today;
    fileCreated = false;
    f_0 = f_4 = f_8 = f_12 = f_16 = f_20 = true;
    arhiv = false;
    Serial.println("♻️ Скидання прапорів — нова дата!");
}
*/
//+++++++++++++++++++++++++++++++++
 /*
 //Test
 if (timeOnly >= "11:12" && flag_test){
 clearOldLogs(3);
  Serial.println("♻️ Очищено архів. Старші 3 днів видалено");
 flag_test=0; 
 }

 //End Test
 */

 /*
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
 //End Test   
 */

//+++++++++++++++++++++++++++++++++
// Заміна 03.08.2025. Архівація та очищення логів кожні 4 години
// ✅ 2. Створення нового CSV файлу о 00:03
 timeOnly = getShortTime();
int hh = timeOnly.substring(0,2).toInt();
int mm = timeOnly.substring(3,5).toInt();
int minutesNow = hh * 60 + mm;

  if (minutesNow >= 3 && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";
    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);  // UTF-8 BOM
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Створено новий архівний файл: " + filename);
      }
    } else {
      Serial.println("ℹ️ Файл архіву вже існує: " + filename);
    }
    fileCreated = true;
    lastInitDate = today;
    Serial.println("♻️ Скидання прапорів — нова дата!");
     f_0 = f_4 = f_8 = f_12 = f_16 = f_20 = true;//Додано 13.08.2025. Скидання прапорів для архівації кожні 4 години
     arhiv = false;//Додано 13.08.2025. Скидання прапорів для архівації кожні 4 години
     //lastDateReset = today;
     //resetHistoryForNewDay();
       // Діагностика флагів
  Serial.print("Значення флага 00: ");
  Serial.println(f_0);
  Serial.print("Значення флага 04: ");
  Serial.println(f_4);
  Serial.print("Значення флага 08: ");
  Serial.println(f_8);
  Serial.print("Значення флага 12: ");
  Serial.println(f_12);
  Serial.print("Значення флага 16: ");
  Serial.println(f_16);
  Serial.print("Значення флага 20: ");
  Serial.println(f_20);
  Serial.println("♻️ Встановлення флагів працює!");
  }

// ✅ 3. Архівація кожні 4 години (23:58, 04:00, 08:00, 12:00, 16:00, 20:00)
// Мій алгоритм архівації


if (minutesNow >= 23*60+58 && f_0) { f_0 = false; arhiv = true; }
if (minutesNow >=  4*60+0  && f_4) { f_4 = false; arhiv = true; }
if (minutesNow >=  8*60+0  && f_8) { f_8 = false; arhiv = true; }
if (minutesNow >= 12*60+0  && f_12) { f_12 = false; arhiv = true; }
if (minutesNow >= 16*60+0  && f_16) { f_16 = false; arhiv = true; } 
if (minutesNow >= 20*60+0  && f_20) { f_20 = false; arhiv = true; }


/*
if (timeOnly >= "23:58" && f_0)  {  f_0 = false; arhiv = true; }
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
if (timeOnly >= "04:00" && f_4)  { f_4 = false; arhiv = true;}
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
if (timeOnly >= "08:00" && f_8)  { f_8 = false; arhiv = true;}
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
if (timeOnly >= "12:00" && f_12) { f_12 = false; arhiv = true;}
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
if (timeOnly >= "16:00" && f_16) { f_16 = false; arhiv = true;} 
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
if (timeOnly >= "20:00" && f_20) { f_20 = false; arhiv = true;} 
   //Serial.print("Флаг архівації: ");
   //Serial.print(arhiv);}
   */
// Якщо архівація активна
if (arhiv) {
  //String archiveDate = (timeOnly.substring(0,2) == "00") ? getTodayDate(-1) : getTodayDate();
  //char *param = strdup(archiveDate.c_str());
  //Serial.printf("📦 Архівація розпочата: %s\n", archiveDate.c_str());

   Serial.printf("⏳ %s: Запускаю архівацію з очищенням масиву...\n", timeOnly.c_str());
  bool *param = new bool(true);  // true = очистити history[] після архівування 
  xTaskCreatePinnedToCore(
    saveHistoryTask,
    "SaveHistory",
    8192,
    param,
    1,
    NULL,
    1
  );
  arhiv = false;
}
 // ==============================
// ⏳ Очікування завершення архівації та очищення логів 25.07.2025
// ==============================
if (!archivingInProgress && clearAfterSaveTriggered) {
  Serial.println("♻️ Очищення старих логів...");
  clearOldLogs(14);  // залишає лише останні 14 дні
  clearAfterSaveTriggered = false;
  Serial.println("✅ Завершено очищення архівів");
}
//+++++++++++++++++++++++++++++++++
  // ==========================================================
  // Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ
  // Цей блок перевіряє, чи минула 5 хвилин (300000 мс) з останнього оновлення даних.
  // Він працює незалежно від того, що відображається на екрані.
  // ==========================================================
  // readBME280Data();  // ✅ Оновлює дані BME280, перезапускає I²C у разі помилки
  // Періодичне зчитування BME280 з інтервалом 5 хв. секунд і контролем коректності даних
  /*
 static uint32_t lastUpdate = 0;
 //uint32_t currentMillis = millis();

 if (currentMillis - lastUpdate > 300000) { // 5 хвилин = 300000 мс
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
  // Діагностика флагів
  Serial.print("Значення флага 00: ");
  Serial.println(f_0);
  Serial.print("Значення флага 04: ");
  Serial.println(f_4);
  Serial.print("Значення флага 08: ");
  Serial.println(f_8);
  Serial.print("Значення флага 12: ");
  Serial.println(f_12);
  Serial.print("Значення флага 16: ");
  Serial.println(f_16);
  Serial.print("Значення флага 20: ");
  Serial.println(f_20);
  // Діагностика памʼяті
    Serial.printf("🧠 Вільно Heap: %u байт, Мін Вільно Heap: %u байт\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
    //Serial.printf("🧠 Вільно Heap: %u байт\n", ESP.getFreeHeap());
    
    
 }
  */
// 📊 Перевірка + дисплей кожні 30 секунд
    if (currentMillis - lastDisplayUpdate > DISPLAY_INTERVAL) {
        lastDisplayUpdate = currentMillis;

        if (checkAndRecoverBME280()) {
            Serial.printf("🌡️ T: %.1f°C, 💧 H: %.1f%%, 📈 P: %.2f Па\n", lastTemp, lastHum, lastPress);
             // displayBMEData(); // Виклик прибрано, щоб він не перебивав інші екрани (відмальовується лише у свій час у ротації)
            //updateDisplay(lastTemp, lastHum, lastPress); // Твоя функція
        } else {
            Serial.println("⚠️ Дані недоступні — дисплей не оновлено");
        }
        averpress = averpress + lastPress;
        avertemperature =  avertemperature + lastTemp;
        averhumidity = averhumidity + lastHum;
        counpress = counpress + 1; // Підрахунок кількості вимірювань тиску
    }

    // 🗃️ Архівація кожні 5 хвилин
    if (currentMillis - lastArchiveUpdate > ARCHIVE_INTERVAL) {
        lastArchiveUpdate = currentMillis;
averpress = averpress / counpress; // Середнє значення тиску
avertemperature = avertemperature / counpress; // Середнє значення температури
averhumidity = averhumidity / counpress; // Середнє значення вологості
counpress = 0; // Скидання лічильника тиску

        if (isBME280DataValid(avertemperature, averhumidity, averpress)) {
        //if (isBME280DataValid(lastTemp, lastHum, averpress)) {  
            String timeStr = getCurrentTime(); // Твоя функція часу
            storeToHistory(avertemperature, averhumidity, averpress, timeStr);
            //storeToHistory(lastTemp, lastHum, averpress, timeStr);
            Serial.println("✅ Дані BME280 збережено в архів");
        } else {
            Serial.println("⚠️ Дані неадекватні — архівація пропущена");
        }
         // Діагностика флагів
  Serial.print("Значення флага 00: ");
  Serial.println(f_0);
  Serial.print("Значення флага 04: ");
  Serial.println(f_4);
  Serial.print("Значення флага 08: ");
  Serial.println(f_8);
  Serial.print("Значення флага 12: ");
  Serial.println(f_12);
  Serial.print("Значення флага 16: ");
  Serial.println(f_16);
  Serial.print("Значення флага 20: ");
  Serial.println(f_20); 
  averpress = 0; // Скидання середнього тиску після архівації
  avertemperature = 0; // Скидання середньої температури після архівації
  averhumidity = 0; // Скидання середньої вологості після архівації
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
//+++++++++++++++++++++++++++++++++
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
  } //End switch (currentDisplayMode)
//+++++++++++++++++++++++++++++++++
} // End loop()

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

// === 🌡️ ФУНКЦІЯ ДОДАВАННЯ ЗАПИСУ ДО HISTORY[] ===
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr) {
  if (historyIndex >= MAX_MEASUREMENTS) return;
 
history[historyIndex].timeStr = timeStr;
history[historyIndex].temperature = temp;
history[historyIndex].humidity = hum;
history[historyIndex].pressure = pres;
historyIndex++;
}
// ===================================================

String getTodayDate(int offset) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    DateTime now = rtc.now();
    time_t rawtime;
    struct tm temp_tm = {0};
    temp_tm.tm_year = now.year() - 1900;
    temp_tm.tm_mon = now.month() - 1;
    temp_tm.tm_mday = now.day();
    temp_tm.tm_hour = now.hour();
    temp_tm.tm_min = now.minute();
    temp_tm.tm_sec = now.second();
    rawtime = mktime(&temp_tm);
    rawtime += offset * 86400; // Зміщення
    localtime_r(&rawtime, &timeinfo);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return String(buffer);
  }

  // Зміщення дати на offset днів
  time_t now = mktime(&timeinfo);
  now += offset * 86400; // 86400 секунд = 1 день
  localtime_r(&now, &timeinfo);

  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

String getTodayDate() {
    return getTodayDate(0);
}


// 🔁 Перезапуск датчика
bool restartBME280() {
    Serial.println("🔄 Перезапуск BME280...");
    digitalWrite(SENSOR_POWER_PIN, LOW);
    delay(100);
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(200);
    return bme.begin(0x76);
}

// 📊 Перевірка даних
bool isBME280DataValid(float temp, float hum, float press) {
    if (isnan(temp) || isnan(hum) || isnan(press)) return false;
    if (temp < -40 || temp > 85) return false;
    if (hum < 0 || hum > 100) return false;
    if (press < 80000 || press > 110000) return false;
    return true;
}

// 🛡️ Перевірка + відновлення
bool checkAndRecoverBME280() {
    float temp = bme.readTemperature() + tempOffset;
    float hum  = bme.readHumidity() + humOffset;
    float press = bme.readPressure();

    if (!isBME280DataValid(temp, hum, press)) {
        Serial.println("⚠️ Дані некоректні — спроба перезапуску...");
        if (!restartBME280()) {
            Serial.println("❌ Перезапуск не допоміг");
            return false;
        }
        delay(500);
        temp = bme.readTemperature() + tempOffset;
        hum  = bme.readHumidity() + humOffset;
        press = bme.readPressure();
        if (!isBME280DataValid(temp, hum, press)) {
            Serial.println("❌ Дані все ще некоректні після перезапуску");
            return false;
        }
    }

    lastTemp = temp;
    lastHum = hum;
    lastPress = press;
    return true;
}
