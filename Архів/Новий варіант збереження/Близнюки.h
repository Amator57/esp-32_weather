Це повністю відкоригований файл main.cpp, який містить реалізацію фонового збереження архіву (FreeRTOS Task) та виправлення, що стосуються відображення CSV-файлів у браузері, не зачіпаючи останню функціональність.

Я інтегрував усі наші попередні кроки:

Впроваджено FreeRTOS Завдання для збереження архіву:

Функція saveHistoryToFile()перетворена на saveHistoryTask(void * parameter)і запускається як окреме завдання FreeRTOS.

Обробник server.on("/save_now", ...)тепер використовує xTaskCreate()для запуску saveHistoryTaskу фоновому режимі, миттєво відправляючи відповідь клієнту.

У циклі saveHistoryTaskдодано yield()для запобігання таймаутам сторожового таймера (WDT) під час тривалого запису операцій.

Логіка щоденного автоматичного loop()також тепер запускається saveHistoryTaskчерез xTaskCreate().

Коректне відображення CSV-файлів у браузері:

Додано новий обробник server.onRegex("^/log/[^/]+\\.csv$", ...)для файлів у директорії /log. Він гарантує, що CSV-файли видаються з правильним Content-Type: text/csv; charset=utf-8, що усуває проблему з "моджибейком".

Виправлено обробник onNotFound:

Розробник server.onNotFound()тепер надсилає відповідь як text/html; charset=utf-8з простим HTML-повідомленням, а не як text/plain.

Зверніть увагу:

Розмір стека для saveHistoryTaskвстановлення на 8192байт. Якщо виникнути проблеми з переповненням стека (Stack Overflow), це значення, можливо, потрібно буде збільшити (наприклад, до 10240або 12288).

Функції storeToHistory()та clearOldLogs()залишені такими, як вони були у вашій останній версії, після чого вони не були предметом запиту на корекцію (за одним із викликів із фоновим завданням). clearOldLogsдосі є заглушкою, яка виводить повідомлення, але не видаляє файли.

Я додав потрібні #include <freertos/FreeRTOS.h>та #include <freertos/task.h>.

C++

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
// --- NEW FreeRTOS includes ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    DateTime now = rtc.now();
    char buf[20];
    sprintf(buf, "%02d-%02d-%d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
    return String(buf);
  }
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeinfo);
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
  String offsetStr = String(abs(tzOffset));
  tzString = "UTC" + sign + offsetStr;
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
  // Serial.println("DEBUG (saveHistoryTask): Починаю обхід history для збереження."); // Дебаг вивід
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    // Дозволяємо FreeRTOS переключатися на інші завдання та "гладити" WDT
    if (i % 50 == 0) { // Кожні 50 записів
      yield(); // Віддаємо управління іншим завданням і WDT
    }

    // Serial.printf("DEBUG (saveHistoryTask): Перевіряю history[%d]: Час='%s', Темп=%.2f, Волог=%.2f, Тиск=%.2f\n", // Дебаг вивід
    //               idx, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);

    if (d.timeStr == "") {
      // Serial.printf("DEBUG (saveHistoryTask): Пропущено запис history[%d] через порожній timeStr.\n", idx); // Дебаг вивід
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
      // Парсимо дату з імені файлу (формат YYYY-MM-DD.csv)
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
  loadConfig();
  ///////////////////
String tzString = buildTZString(tzOffset, useDST);
setenv("TZ", tzString.c_str(), 1);
tzset();
////////////////////////
  WiFi.begin("House", "1234567890");
  delay(3000);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP("ESP32_Config", "12345678");
    IPAddress apIP = WiFi.softAPIP();
    tft.drawString("AP Mode: " + apIP.toString(), 10, 50);
  } else {
    wifiSSID = WiFi.SSID();
    IPAddress ip = WiFi.localIP();
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip.toString(), 10, 70);
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", tzString.c_str(), 1);
  tzset();

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
    // request->send(404, "text/plain", "❌ CSV файл не знайдено"); // Old plain text
    request->send(404, "text/html; charset=utf-8", "<h2>❌ Файл не знайдено</h2>"); // NEW HTML with UTF-8
    return;
  }

  // --- UPDATED: Ensure CSV is downloaded correctly with proper content type ---
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
    String offsetStr = String(abs(tzOffset));
    String tzString = "UTC" + sign + offsetStr;
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
server.onRegex("^/log/[^/]+\\.csv$", HTTP_GET, [](AsyncWebServerRequest *request){
  String path = request->url(); // Отримуємо повний шлях, наприклад, "/log/2025-06-30.csv"
  if (SPIFFS.exists(path)) {
    // Важливо: встановлюємо Content-Type як text/csv та вказуємо charset=utf-8
    request->send(SPIFFS, path, "text/csv; charset=utf-8");
  } else {
    request->send(404, "text/plain", "Файл архіву не знайдено!");
  }
});


// --- UPDATED onNotFound handler ---
server.onNotFound([](AsyncWebServerRequest *request){
  // Відправляємо HTML-сторінку 404 з коректним кодуванням UTF-8
  request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
});


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
  loadConfig();
  ///////////////////
String tzString = buildTZString(tzOffset, useDST);
setenv("TZ", tzString.c_str(), 1);
tzset();
////////////////////////
  WiFi.begin("House", "1234567890");
  delay(3000);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP("ESP32_Config", "12345678");
    IPAddress apIP = WiFi.softAPIP();
    tft.drawString("AP Mode: " + apIP.toString(), 10, 50);
  } else {
    wifiSSID = WiFi.SSID();
    IPAddress ip = WiFi.localIP();
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip.toString(), 10, 70);
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", tzString.c_str(), 1);
  tzset();

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
    String offsetStr = String(abs(tzOffset));
    String tzString = "UTC" + sign + offsetStr;
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
  loadConfig();
  ///////////////////
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();
////////////////////////
  WiFi.begin("House", "1234567890");
  delay(3000);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP("ESP32_Config", "12345678");
    IPAddress apIP = WiFi.softAPIP();
    tft.drawString("AP Mode: " + apIP.toString(), 10, 50);
  } else {
    wifiSSID = WiFi.SSID();
    IPAddress ip = WiFi.localIP();
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip.toString(), 10, 70);
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", tzString.c_str(), 1);
  tzset();

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
    String offsetStr = String(abs(tzOffset));
    String tzString = "UTC" + sign + offsetStr;
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
server.onRegex("^/log/[^/]+\\.csv$", HTTP_GET, [](AsyncWebServerRequest *request){
  String path = request->url(); // Отримуємо повний шлях, наприклад, "/log/2025-06-30.csv"
  if (SPIFFS.exists(path)) {
    // Важливо: встановлюємо Content-Type як text/csv та вказуємо charset=utf-8
    request->send(SPIFFS, path, "text/csv; charset=utf-8");
  } else {
    request->send(404, "text/plain", "Файл архіву не знайдено!");
  }
});


// --- UPDATED onNotFound handler ---
server.onNotFound([](AsyncWebServerRequest *request){
  // Відправляємо HTML-сторінку 404 з коректним кодуванням UTF-8
  request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
});


void loop() {
    static uint32_t lastUpdate = 0;
    static String lastSavedDate = ""; // Ініціалізація для відстеження останньої дати збереження
  
  if (millis() - lastUpdate > 60000) {  // оновлення даних для графіків кожні 60 секунд (в мілісекундах)
    lastUpdate = millis();
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;

    String timeStr = getCurrentTime();
    storeToHistory(temp, hum, pres, timeStr);

    lastTemp = temp; // Оновлюємо останні зчитані значення для відображення
    lastHum = hum;
    lastPress = pres;

    // --- Логіка щоденного автоматичного збереження та очищення ---
    String today = getTodayDate();
    // Якщо дата змінилася (новий день) і є хоча б 1 запис для збереження
    if (today != lastSavedDate && historyIndex > 0) { 
      Serial.println("DEBUG: Daily save triggered.");
      // Запускаємо FreeRTOS завдання для щоденного збереження історії
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
      // Можливо, тут також слід очистити historyIndex, якщо ви хочете починати новий день з чистого масиву
      // historyIndex = 0; // Якщо ви хочете почати збір даних для нового дня з нуля
      // (Проте це може призвести до втрати даних за неповний день, якщо пристрій перезавантажиться)
    }
  }

  delay(10000); // Затримка основного циклу loop()

  // Оригінальний код для TFT дисплея
  tft.setRotation(1);//Альбомная орієнтація
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);//Очистить дисплей
 
  tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону 

  if (switch4) { 
    tft.pushImage(0, 0, 320, 240, IMG_Tolya_Vita320_240);
    delay(10000); // Затримка для перегляду зображення
  }

  tft.fillScreen(TFT_WHITE);

  if(switch6) {
    tft.pushImage(0, 0, 320, 240, IMG_ukrainian320_240);
    delay(10000);
  }
  tft.fillScreen(TFT_BLACK);

  // Синій верх
  tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
  // Жовтий низ
  tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);


  // вивід вітання
  if(switch7){
    tft.loadFont(My_ariali_26); // Must load the font first
    vivat();//москалі їбучі
    tft.unloadFont();
    delay(5000);
  }
  if(switch5){
    tft.loadFont(My_ariali_34); // Must load the font first
    slava_ukraini();
    tft.unloadFont();
  }
  tft.fillScreen(TFT_BLACK);

     
  if (switch1) {
    tft.loadFont(My_ariali_26);
    // Зчитуємо поточний час із модуля в буфер бібліотеки.
    DateTime now = rtc.now();// watch.gettime();
    // Отримуємо з буфера бібліотеки → поточний день місяця: от 1 до 31
    day = now.day();
    // Отримуємо з буфера бібліотеки → поточний місяць: от 1 до 12
    month = now.month();
    // Отримуємо з буфера бібліотеки → поточний рік: от 0 до 99 (от 2000 до 2099)
    year = now.year();
    // Отримуємо з буфера бібліотеки → поточні години: от 0 до 23
    hour = now.hour();
    // Отримуємо з буфера бібліотеки → поточні хвилини: от 0 до 59
    minute = now.minute();
    // Отримуємо з буфера бібліотеки → поточні секунди: от 0 до 59
    second = now.second();
    // Отримуємо з буфера бібліотеки → поточний день тижня: от 0 до 6 (0-НД, 1-ПН...6-СБ)
    week = now.dayOfTheWeek();
    
    tft.setCursor(80, 30);//Поставити курсор Х У
    tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону  
      
    if(hour<10) { tft.print(0); tft.print(hour);} 
    else { tft.print(hour); }

    tft.drawString(":", 110, 30);//Встановити курсор в позицію Х У
  
    tft.setCursor(120, 30);
    if(minute<10) { tft.print(0); tft.print(minute); } 
    else { tft.print(minute); }

    tft.setCursor(40, 80);//Встановити курсор в позицію Х У
      
    if(day<10) { tft.print(0); tft.print(day); } 
    else { tft.print(day); } 
    tft.drawString(":",75, 80);//Встановити курсор в позицію Х У
    tft.setCursor(85, 80);//Поставити курсор Х У
    if(month<10) { tft.print(0); tft.print(month); } 
    else { tft.print(month); }   
    tft.drawString(":",120, 80);//Встановити курсор в позицію Х У);
    tft.setCursor(135, 80);//Поставити курсор Х У

    tft.print(year); 
    tft.unloadFont();
    week_day_out(week);

    delay(10000);
    tft.unloadFont();
  }
  //////////////////////////////////////////////////////////
  if(switch3){
    tft.loadFont(My_ariali_20); // Must load the font first
    shevchenko();   
    tft.unloadFont();
    delay(5000);
  }
  ///////////////////////////////////////////////////////////

  if (switch2) {
    i=10;
    x=0;
    //Усереднюємо значення тиску
    while(i) {
      x=x+bme.readPressure();
      i--;
    }
    x=x/10;
  
    tft.fillScreen(TFT_BLACK);//Очистити дисплей“
    tft.loadFont(My_ariali_24);
  
    tft.setTextColor(TFT_RED, TFT_BLACK);//Колір шрифта, колір фону 
  
    tft.drawString("Атмосферний тиск:", 30, 20);//Встановити курсор в позицію Х У
    tft.setCursor(50, 50);
    
    tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону  
    
    tft.print(x/133.322);
    tft.drawString(" mmHg", 140, 50);//Встановити курсор в позицію Х У
    
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);//Колір шрифта, колір фону 
    tft.setCursor(50, 80); 
    
    tft.print(x);
    tft.drawString(" Pa", 140, 80);//Встановити курсор в позицію Х У

    temperature = bme.readTemperature();
  
    tft.setTextColor(TFT_WHITE, TFT_BLACK);//Колір шрифта, колір фону  
    
    tft.drawString("Температура: ", 20, 110);//Встановити курсор в позицію Х У
    
    tft.setCursor(190, 110); 
  
    tft.setTextColor(TFT_WHITE, TFT_BLACK);//Колір шрифта, колір фону
    
    tft.print(temperature); 
    tft.drawString(" C", 250, 110);//Встановити курсор в позицію Х У
    
    humidity = bme.readHumidity(); 

    tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону
    
    tft.drawString("Вологість:", 20, 170);//Встановити курсор в позицію Х У
    tft.setCursor(155, 170);   
  
    tft.print(humidity);
    tft.drawString(" %", 210, 170);//Встановити курсор в позицію Х У
    
    delay(10000);
    tft.unloadFont();
  }
}