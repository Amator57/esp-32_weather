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

bool switch1 = false, switch2 = false, switch3 = false, switch4 = false;
bool switch5 = false, switch6 = false, switch7 = false;
float lastTemp = 0, lastHum = 0, lastPress = 0;
float temperature;
float pressure;
float humidity;
int8_t   hour, minute, second;
long x;//усереднення тиску
unsigned char i;//усереднення тиску
unsigned char h;//змінна вологості
unsigned char q = 0;//змінна входження в режим встановлення
unsigned char v = 0;
unsigned char s = 0;

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
  tft.setRotation(1);
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
}


void loop() {
    static uint32_t lastBMEUpdate = 0;
    static uint32_t lastDisplayUpdate = 0;
    static String lastSavedDate = ""; // Ініціалізація для відстеження останньої дати збереження
    const uint32_t BME_UPDATE_INTERVAL = 60000; // 60 секунд
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
                delay(100); // Короткий delay, щоб не блокувати
            }
            // (можливо, тут потрібно очистити екран після зображення, якщо воно не займає весь екран)

            tft.fillScreen(TFT_BLACK); // Заповнюємо екран чорним перед новим малюнком

            if (switch6) {
                tft.pushImage(0, 0, 320, 240, IMG_ukrainian320_240);
                delay(100);
            }
            tft.fillScreen(TFT_BLACK);

            tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
            tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);

            if (switch7) {
                tft.loadFont(My_ariali_26);
                vivat();
                tft.unloadFont();
                delay(100);
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
                delay(100);
            }

            if (switch3) {
                tft.loadFont(My_ariali_20);
                shevchenko();
                tft.unloadFont();
                delay(100);
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
                delay(100);
            }
        }
    }
    // Дуже важлива функція для FreeRTOS, дозволяє іншим задачам працювати.
    delay(10); // Коротка затримка, щоб дозволити іншим завданням (напр., веб-серверу) працювати
}