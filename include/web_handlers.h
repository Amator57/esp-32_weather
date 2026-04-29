#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// === ✅ Глобальні об’єкти (якщо потрібно в main.cpp – коментуєш дублікати там) ===
extern AsyncWebServer server;
extern Preferences preferences; // Один єдиний об'єкт Preferences

// === 🔧 Конфігурація і змінні ===
constexpr int MAX_POINTS = 1440;   // або твоє реальне значення
extern String deviceName;
extern String wifiSSID;
extern String wifiPassword;
extern int tzOffset;
extern bool useDST;
extern bool switch1, switch2, switch3, switch4, switch5, switch6, switch7;
extern float tempOffset;   // ✅ Поправка температури
extern float humOffset;  // поправка вологості
extern String staticIP;
extern String gateway;
extern String subnet;

// === ✅ Оголошення зовнішніх змінних, які використовуються у хендлерах ===
extern float lastTemp, lastHum, lastPress;
extern int historyIndex;
extern const int MAX_POINTS;
/*
struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};
*/
extern BMEData history[];

// === ✅ Допоміжні функції (повинні бути оголошені у main.cpp) ===
String processor(const String& var);
void saveConfig();
void saveHistoryTask(void *parameter);

// ===================================================================================
// 🛠️ Завантаження налаштувань з NVS
// ===================================================================================
void loadConfig() { 
  preferences.begin("config", false); // false = читання/запис
  deviceName = preferences.getString("deviceName", deviceName);
  wifiSSID   = preferences.getString("wifiSSID", wifiSSID);
  wifiPassword = preferences.getString("wifiPassword", wifiPassword);
  
  // Міграція зі старого ключа "tzOffset" на новий "tz_off"
  if (preferences.isKey("tz_off")) {
      tzOffset = preferences.getInt("tz_off", 2);
  } else if (preferences.isKey("tzOffset")) {
      tzOffset = preferences.getInt("tzOffset", 2);
      Serial.println("ℹ️ Міграція ключа: tzOffset -> tz_off");
      preferences.putInt("tz_off", tzOffset);
  } else {
      tzOffset = 2;
  }

  useDST     = preferences.getBool("use_dst_b", true);
  switch1 = preferences.getBool("switch1", false);
  switch2 = preferences.getBool("switch2", false);
  switch3 = preferences.getBool("switch3", false);
  switch4 = preferences.getBool("switch4", false);
  switch5 = preferences.getBool("switch5", false);
  switch6 = preferences.getBool("switch6", false);
  switch7 = preferences.getBool("switch7", false);
  tempOffset = preferences.getFloat("tempOffset", 0.0);
  humOffset  = preferences.getFloat("humOffset", 0.0);
  staticIP   = preferences.getString("staticIP", "192.168.1.230");
  gateway    = preferences.getString("gateway", "192.168.1.1");
  subnet     = preferences.getString("subnet", "255.255.255.0");
  
  Serial.printf("🌡️ Температурний офсет: %.1f°C\n", tempOffset);
  Serial.printf("💧 Вологісний офсет: %.1f%%\n", humOffset);
  Serial.printf("🕒 Часовий пояс (UTC): %d (DST: %s)\n", tzOffset, useDST ? "Так" : "Ні");
  preferences.end();

  // Встановлення часової зони (EET формат для нашого регіону)
  String sign = tzOffset >= 0 ? "-" : "+";
  int absOffset = abs(tzOffset);
  // Використовуємо EET/EEST для зміщення 2, для інших - загальний формат TZN
  String zoneName = (tzOffset == 2) ? "EET" : "TZN";
  String dstName  = (tzOffset == 2) ? "EEST" : "TZD";
  
  String tzStr = zoneName + sign + String(absOffset);
  if (useDST) {
    tzStr += dstName + ",M3.5.0/3,M10.5.0/4";
  }
  setenv("TZ", tzStr.c_str(), 1);
  tzset();
  Serial.printf("🌍 Системна часова зона (TZ) встановлена: %s\n", tzStr.c_str());
}

// ===================================================================================
// 📄 Головна сторінка (index.html)
// ===================================================================================
static void handleRootPage(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
}

// ===================================================================================
// 📄 Сторінка налаштувань (settings.html)
// ===================================================================================
static void handleSettingsPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
}

// ===================================================================================
// 📊 Список архівів (/logs)
// ===================================================================================
static void handleLogs(AsyncWebServerRequest *request) {
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
            if (!fullPath.endsWith(".csv")) {
                file = root.openNextFile();
                continue;
            }
            String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
            String datePart = fileName;
            datePart.replace(".csv", "");

            response->print("<li>");
            response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
            response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
            response->print("</li>");

            file = root.openNextFile();
        }
    }
    response->print(F("</ul><p><a href=\"/all_logs_viewer\">📈 Переглянути всі дні разом (Зведені графіки)</a></p><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
}

// ===================================================================================
// 📊 Перегляд окремого логу (log_viewer.html)
// ===================================================================================
static void handleLogViewer(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
}

// ===================================================================================
// 📊 API: Список логів (/api/logs)
// ===================================================================================
static void handleApiLogs(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");
    File root = SPIFFS.open("/log");
    bool first = true;
    if (root && root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name();
            if (fullPath.endsWith(".csv")) {
                String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
                if (!first) response->print(",");
                response->print("\"" + fileName + "\"");
                first = false;
            }
            file = root.openNextFile();
        }
    }
    response->print("]");
    request->send(response);
}

// ===================================================================================
// 📊 Перегляд зведеного архіву (all_logs_viewer.html)
// ===================================================================================
static void handleAllLogsViewer(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/all_logs_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
}

// ===================================================================================
// 🌡️ API: Поточні дані з BME280 (/bme_data)
// ===================================================================================
static void handleBMEData(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = lastTemp + tempOffset;   // ✅ поправка врахована
    doc["humidity"]    = lastHum;
    doc["pressure"]    = lastPress / 100.0F;
    doc["presmmhg"]    = lastPress / 133.322F;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

// ===================================================================================
// 🌡️ API: Графік поточних даних (/bme_chart_data)
// ===================================================================================
static void handleBMEChartData(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");

    bool first = true;
    for (int i = 0; i < MAX_POINTS; i++) {
        int idx = (historyIndex + i) % MAX_POINTS;
        const BMEData &d = history[idx];
        if (d.timeStr == "") continue;

        if (!first) response->print(",");
        first = false;

        response->printf(
            "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
            d.timeStr, d.temperature + tempOffset, d.humidity, d.pressure
        );
    }
    response->print("]");
    request->send(response);
}

// ===================================================================================
// 📂 Ручне збереження (save_now)
// ===================================================================================
static void handleManualSave(AsyncWebServerRequest *request) {
    bool *param = new bool(false);  // ручне збереження (без очищення)
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);
    request->send(200, "text/plain", "✅ Ручне збереження розпочато у фоновому режимі.");
}

// ===================================================================================
// 🔁 Рестарт ESP32 (/restart)
// ===================================================================================
static void handleRestart(AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><head><meta charset='UTF-8'><title>Перезавантаження</title>"
        "<script>setTimeout(()=>{window.location.href='/'},8000);</script></head>"
        "<body><h2>🔁 Пристрій перезавантажується...</h2></body></html>");
    delay(500);
    ESP.restart();
}

// ===================================================================================
// ✅ Реєстрація всіх хендлерів
// ===================================================================================
void registerWebHandlers(AsyncWebServer &server) {
    server.on("/", HTTP_GET, handleRootPage);
    server.on("/settings", HTTP_GET, handleSettingsPage);
    server.on("/logs", HTTP_GET, handleLogs);
    server.on("/api/logs", HTTP_GET, handleApiLogs);
    server.on("/log_viewer", HTTP_GET, handleLogViewer);
    server.on("/all_logs_viewer", HTTP_GET, handleAllLogsViewer);
    server.on("/bme_data", HTTP_GET, handleBMEData);
    server.on("/bme_chart_data", HTTP_GET, handleBMEChartData);
    server.on("/save_now", HTTP_GET, handleManualSave);
    server.on("/restart", HTTP_POST, handleRestart);

    // --- Статичні файли ---
    server.serveStatic("/log/", SPIFFS, "/log/")
          .setCacheControl("no-store, no-cache, must-revalidate");

    // --- 404 Not Found ---
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
    });
}

/*
#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// === ✅ Глобальні об’єкти ===
extern AsyncWebServer server;
extern Preferences preferences; // Один єдиний об'єкт Preferences

constexpr int MAX_POINTS = 1440;   // або твоє реальне значення

// === 🔧 Глобальні змінні конфігурації ===
extern String deviceName;
extern String wifiSSID;
extern int tzOffset;
extern bool useDST;
extern bool switch1, switch2, switch3, switch4, switch5, switch6, switch7;

// ➕ Додано
extern float tempOffset;  // Поправка температури (°C)

// === ✅ Оголошення зовнішніх змінних ===
extern float lastTemp, lastHum, lastPress;
extern int historyIndex;
extern const int MAX_POINTS;

struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

extern BMEData history[];

// === ✅ Допоміжні функції ===
String processor(const String& var);
void saveConfig();
void saveHistoryTask(void *parameter);

// ===================================================================================
// 🛠️ ФУНКЦІЯ ЗАВАНТАЖЕННЯ НАЛАШТУВАНЬ
// ===================================================================================
void loadConfig() { 
  preferences.begin("config", true);
  deviceName  = preferences.getString("deviceName", deviceName);
  wifiSSID    = preferences.getString("wifiSSID", wifiSSID);
  tzOffset    = preferences.getInt("tzOffset", 2);
  useDST      = preferences.getBool("useDST", true);

  switch1 = preferences.getBool("switch1", false);
  switch2 = preferences.getBool("switch2", false);
  switch3 = preferences.getBool("switch3", false);
  switch4 = preferences.getBool("switch4", false);
  switch5 = preferences.getBool("switch5", false);
  switch6 = preferences.getBool("switch6", false);
  switch7 = preferences.getBool("switch7", false);

  // ➕ Завантаження поправки
  tempOffset = preferences.getFloat("tempOffset", 0.0f);

  preferences.end();

  // Встановлення часової зони
  String sign = tzOffset >= 0 ? "-" : "+";
  int absOffset = abs(tzOffset);
  String tzString = "UTC" + sign + String(absOffset);
  if (useDST) {
    tzString += "DST,M3.5.0/3,M10.5.0/4";
  }
  setenv("TZ", tzString.c_str(), 1);
  tzset();
}

// ===================================================================================
// 📄 СТОРІНКА НАЛАШТУВАНЬ
// ===================================================================================
static void handleSettingsPage(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<h2>Налаштування</h2>");

    // ➕ Калібрування температури
    response->print("<h3>Калібрування температури</h3>");
    response->print("<form action='/setTempOffset' method='GET'>");
    response->print("Поправка (°C): <input type='number' step='0.1' name='offset' value='" + String(tempOffset, 1) + "'>");
    response->print("<input type='submit' value='Зберегти'></form>");

    response->print("<p><a href='/'>На головну</a></p>");
    request->send(response);
}

// ===================================================================================
// 🌡️ API: Поточні дані з BME280
// ===================================================================================
static void handleBMEData(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = lastTemp + tempOffset;   // ➕ застосовуємо поправку
    doc["humidity"]    = lastHum;
    doc["pressure"]    = lastPress / 100.0F;
    doc["presmmhg"]    = lastPress / 133.322F;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

// ===================================================================================
// ✅ РЕЄСТРАЦІЯ ВСІХ ХЕНДЛЕРІВ
// ===================================================================================
void registerWebHandlers(AsyncWebServer &server) {
    server.on("/", HTTP_GET, handleRootPage);
    server.on("/settings", HTTP_GET, handleSettingsPage);

    // ➕ Хендлер збереження поправки
    server.on("/setTempOffset", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (request->hasParam("offset")) {
        tempOffset = request->getParam("offset")->value().toFloat();
        preferences.begin("config", false);
        preferences.putFloat("tempOffset", tempOffset);
        preferences.end();
        Serial.printf("✅ Збережено поправку температури: %.1f °C\n", tempOffset);
      }
      request->redirect("/settings");
    });

    server.on("/logs", HTTP_GET, handleLogs);
    server.on("/log_viewer", HTTP_GET, handleLogViewer);
    server.on("/bme_data", HTTP_GET, handleBMEData);
    server.on("/bme_chart_data", HTTP_GET, handleBMEChartData);
    
    server.on("/save_now", HTTP_GET, handleManualSave);
    server.on("/restart", HTTP_POST, handleRestart);

    server.serveStatic("/log/", SPIFFS, "/log/").setCacheControl("no-store, no-cache, must-revalidate");

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
    });
}

*/

/*
//оригінальний працюючий варіант web_handlers.h
#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// === ✅ Глобальні об’єкти (якщо потрібно в main.cpp – коментуєш дублікати там) ===
extern AsyncWebServer server;
extern Preferences preferences; // Один єдиний об'єкт Preferences
// У верхній частині web_handlers.h
constexpr int MAX_POINTS = 1440;   // або твоє реальне значення
// === 🔧 Глобальні змінні конфігурації ===
extern String deviceName;
extern String wifiSSID;
extern int tzOffset;
extern bool useDST;
extern bool switch1, switch2, switch3, switch4, switch5, switch6, switch7;

// === ✅ Оголошення зовнішніх змінних, які використовуються у хендлерах ===
extern float lastTemp, lastHum, lastPress;
extern int historyIndex;
extern const int MAX_POINTS;

struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

extern BMEData history[];

// === ✅ Допоміжні функції (повинні бути оголошені у main.cpp) ===
String processor(const String& var);
void saveConfig();
void saveHistoryTask(void *parameter);

// ===================================================================================
// 🛠️ ФУНКЦІЯ ЗАВАНТАЖЕННЯ НАЛАШТУВАНЬ (Preferences → глобальні змінні)
// ===================================================================================
void loadConfig() { 
  preferences.begin("config", true);
  deviceName = preferences.getString("deviceName", deviceName);
  wifiSSID   = preferences.getString("wifiSSID", wifiSSID);
  tzOffset   = preferences.getInt("tzOffset", 2);
  useDST     = preferences.getBool("useDST", true);

  switch1 = preferences.getBool("switch1", false);
  switch2 = preferences.getBool("switch2", false);
  switch3 = preferences.getBool("switch3", false);
  switch4 = preferences.getBool("switch4", false);
  switch5 = preferences.getBool("switch5", false);
  switch6 = preferences.getBool("switch6", false);
  switch7 = preferences.getBool("switch7", false);
  preferences.end();

  // Встановлення часової зони (TZ)
  String sign = tzOffset >= 0 ? "-" : "+";
  int absOffset = abs(tzOffset);
  String tzString = "UTC" + sign + String(absOffset);
  if (useDST) {
    tzString += "DST,M3.5.0/3,M10.5.0/4";
  }
  setenv("TZ", tzString.c_str(), 1);
  tzset();
}

// ===================================================================================
// 📄 ГОЛОВНА СТОРІНКА (index.html)
// ===================================================================================
static void handleRootPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

// ===================================================================================
// 📄 СТОРІНКА НАЛАШТУВАНЬ (settings.html)
// ===================================================================================
static void handleSettingsPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
}

// ===================================================================================
// 📊 СПИСОК АРХІВІВ (/logs)
// ===================================================================================
static void handleLogs(AsyncWebServerRequest *request) {
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
            if (!fullPath.endsWith(".csv")) {
                file = root.openNextFile();
                continue;
            }
            String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
            String datePart = fileName;
            datePart.replace(".csv", "");

            response->print("<li>");
            response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
            response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
            response->print("</li>");

            file = root.openNextFile();
        }
    }
    response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
}

// ===================================================================================
// 📊 ПЕРЕГЛЯД ОКРЕМОГО ЛОГУ (log_viewer.html)
// ===================================================================================
static void handleLogViewer(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
}

// ===================================================================================
// 🌡️ API: Поточні дані з BME280 (/bme_data)
// ===================================================================================
static void handleBMEData(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = lastTemp;
    doc["humidity"]    = lastHum;
    doc["pressure"]    = lastPress / 100.0F;
    doc["presmmhg"]    = lastPress / 133.322F;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

// ===================================================================================
// 🌡️ API: Графік поточних даних (/bme_chart_data)
// ===================================================================================
static void handleBMEChartData(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");

    bool first = true;
    for (int i = 0; i < MAX_POINTS; i++) {
        int idx = (historyIndex + i) % MAX_POINTS;
        const BMEData &d = history[idx];
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
}

// ===================================================================================
// 📂 РУЧНЕ ЗБЕРЕЖЕННЯ (save_now)
// ===================================================================================
static void handleManualSave(AsyncWebServerRequest *request) {
    bool *param = new bool(false);  // ручне збереження (без очищення)
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);
    request->send(200, "text/plain", "✅ Ручне збереження розпочато у фоновому режимі.");
}

// ===================================================================================
// 🔁 РЕСТАРТ ESP32 (/restart)
// ===================================================================================
static void handleRestart(AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><head><meta charset='UTF-8'><title>Перезавантаження</title>"
        "<script>setTimeout(()=>{window.location.href='/'},8000);</script></head>"
        "<body><h2>🔁 Пристрій перезавантажується...</h2></body></html>");
    delay(500);
    ESP.restart();
}

// ===================================================================================
// ✅ РЕЄСТРАЦІЯ ВСІХ ХЕНДЛЕРІВ
// ===================================================================================
void registerWebHandlers(AsyncWebServer &server) {
    server.on("/", HTTP_GET, handleRootPage);
    server.on("/settings", HTTP_GET, handleSettingsPage);
    server.on("/logs", HTTP_GET, handleLogs);
    server.on("/api/logs", HTTP_GET, handleApiLogs);
    server.on("/log_viewer", HTTP_GET, handleLogViewer);
    server.on("/all_logs_viewer", HTTP_GET, handleAllLogsViewer);
    server.on("/bme_data", HTTP_GET, handleBMEData);
    server.on("/bme_chart_data", HTTP_GET, handleBMEChartData);
    server.on("/save_now", HTTP_GET, handleManualSave);
    server.on("/restart", HTTP_POST, handleRestart);
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // відправляємо index.html із SPIFFS
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
});

    // --- Статичні файли ---
    server.serveStatic("/log/", SPIFFS, "/log/")
          .setCacheControl("no-store, no-cache, must-revalidate");

    // --- 404 Not Found ---
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
    });
}

*/
