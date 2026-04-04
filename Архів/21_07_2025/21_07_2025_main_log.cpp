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
// 🔐 Макрос та функція safeAbort для логування перед crash
#define SAFE_ABORT(msg) safeAbort(__FILE__, __LINE__, msg)

void safeAbort(const char* file, int line, const char* msg = "") {
  Serial.printf("\n🚨 АВАРІЙНЕ ЗАВЕРШЕННЯ\nФайл: %s\nРядок: %d\nПричина: %s\n", file, line, msg);
  delay(200);  // ⏳ Забезпечити виведення в монітор
  abort();     // ⛔ Аварійне завершення
}
AsyncWebServer server(80);
Preferences preferences;

String deviceName = "ESP32_Weather";
String wifiSSID = "Unknown";
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
int8_t   hour, minute, second;
long x;//усереднення тиску
// === 🔧 ЗМІННІ ДЛЯ ІНДЕКСУВАННЯ АРХІВАЦІЇ ===
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
void performHistoryFileSave();// Оголошення нової функції для збереження історії

void resetHistoryForNewDay();
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr);
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

//++++++++++++++++++++++++++++++++++++++
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
  WiFi.begin("House", "123123123");
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
  /*
//Test
if (timeOnly >= "09:10" && flag_test){
 clearOldLogs(3);
  Serial.println("♻️ Очищено архів. Старші 3 днів видалено");
 flag_test=0; 
}

//End Test
*/
 // 🧪 Тестовий автозапуск архівації
  //String timeOnly = getCurrentTime().substring(11); // "HH:MM:SS"
  if (timeOnly >= "15:10" && testArchiveFlag) {
    Serial.println("🧪 Тестовий запуск архівації з очищенням архівів...");

    bool *param = new bool(true);  // true = автоматична архівація
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    testArchiveFlag = false;
  }

  //End Test

// ♻️ Скидання флагів один раз на початку нової доби (після 00:01 і до 23:58)
static String lastDateReset = "";
if (timeOnly >= "00:01" && timeOnly < "23:58" && today != lastDateReset) {
  Serial.println("♻️ Скидання флагів архівації та створення — нова дата!");
  archiveDoneToday = false;
  fileCreated = false;
  lastDateReset = today;
  resetHistoryForNewDay(); // скидаємо індекс історії на нуль
}
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
  if (historyIndex >= maxHistorySize) return;
 
history[historyIndex].timeStr = timeStr;
history[historyIndex].temperature = temp;
history[historyIndex].humidity = hum;
history[historyIndex].pressure = pres;
historyIndex++;

}
