
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

#include "Tolya_Vita320_240.h"
#include "ukrainian320_240.h"

#include "globals.h"
#include "text_data.h"
#include "draw_utils.h"
#include "subroutines.h"
// --- NEW FreeRTOS includes ---
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
static String lastSavedDate = "";

String getTodayDate();
void saveHistoryToFile();
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
/*
void storeToHistory(float temp, float hum, float pres, String timeStr) {
  history[historyIndex] = { temp, hum, pres, timeStr };
  historyIndex = (historyIndex + 1) % MAX_POINTS;
}
*/
//Перевірка
void storeToHistory(float temp, float hum, float pres, String timeStr) {
  history[historyIndex] = { temp, hum, pres, timeStr };

  //Serial.printf("DEBUG (storeToHistory): Зберігаю у history[%d]: Час='%s', Темп=%.2f\n",
               // historyIndex, history[historyIndex].timeStr.c_str(), history[historyIndex].temperature); // ДОДАЙТЕ ЦЕЙ РЯДОК
 // ОНОВЛЕНИЙ РЯДОК ДЛЯ ВІДЛАДКИ:
  Serial.printf("DEBUG (storeToHistory): Зберігаю у history[%d]: Час='%s', Темп=%.2f, Волог=%.2f, Тиск=%.2f\n",
                historyIndex,
                history[historyIndex].timeStr.c_str(),
                history[historyIndex].temperature,
                history[historyIndex].humidity,   // Додано вологість
                history[historyIndex].pressure);  // Додано тиск

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
// --- Отримати сьогоднішню дату у форматі YYYY-MM-DD ---
String getTodayDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "unknown";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}
//=========відладочний
// --- Зберегти поточні 1440 записів у файл SPIFFS ---
void saveHistoryToFile() {
  String filename = String("/log/") + getTodayDate() + ".csv";

  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    return;
  }

  // ✅ Додаємо BOM для правильної кирилиці
  file.write(0xEF); file.write(0xBB); file.write(0xBF);

  // CSV заголовок
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  Serial.println("DEBUG (saveHistoryToFile): Починаю обхід history для збереження.");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

   // Serial.printf("DEBUG (saveHistoryToFile): Перевіряю history[%d].timeStr = '%s'\n", idx, d.timeStr.c_str());
 Serial.printf("DEBUG (saveHistoryToFile): Перевіряю history[%d]: Час='%s', Темп=%.2f, Волог=%.2f, Тиск=%.2f\n",
                  idx,
                  d.timeStr.c_str(),
                  d.temperature,
                  d.humidity,    // Додано вологість
                  d.pressure);   // Додано тиск
    if (d.timeStr == "") {
      Serial.printf("DEBUG (saveHistoryToFile): Пропущено запис history[%d] через порожній timeStr.\n", idx);
      continue;
    }

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів збережено: %s (%d записів)\n", filename.c_str(), written);
}
//===========відладка
/*
// --- Зберегти поточні 1440 записів у файл SPIFFS ---
void saveHistoryToFile() {
  String filename = String("/log/") + getTodayDate() + ".csv";

  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    return;
  }

  // ✅ Додаємо BOM для правильної кирилиці
  file.write(0xEF); file.write(0xBB); file.write(0xBF);

  // CSV заголовок
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;
    //========================
    
//перевірка
   int written = 0;
  Serial.println("DEBUG (saveHistoryToFile): Починаю обхід history для збереження."); // ДОДАЙТЕ ЦЕ
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    Serial.printf("DEBUG (saveHistoryToFile): Перевіряю history[%d].timeStr = '%s'\n", idx, d.timeStr.c_str()); // ДОДАЙТЕ ЦЕЙ РЯДОК

    if (d.timeStr == "") {
      Serial.printf("DEBUG (saveHistoryToFile): Пропущено запис history[%d] через порожній timeStr.\n", idx); // ДОДАЙТЕ ЦЕЙ РЯДОК
      continue;
    }

    //++++++++++++++++++++++++

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів збережено: %s (%d записів)\n", filename.c_str(), written);
}
*/
/*
void saveHistoryToFile() {
  //String filename = "/log/" + getCurrentDate() + ".csv";  // Наприклад: /log/2025-06-28.csv
  String filename = String("/log/") + getTodayDate() + ".csv";
  // Створюємо теку, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);  // Перезапис
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    return;
  }

  file.println("Time,Temperature,Humidity,Pressure");  // Заголовок CSV

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;  // пропустити порожні записи

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів збережено: %s (%d записів)\n", filename.c_str(), written);
}


void saveHistoryToFile() {
  String today = getTodayDate(); // формату "2025-06-28"
  String path = "/log/" + today + ".csv";

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    return;
  }

  file.println("Час,Температура,Вологість,Тиск");

  // Зберігаємо лише дійсні значення
  int count = (historyIndex < MAX_POINTS) ? historyIndex : MAX_POINTS;
  for (int i = 0; i < count; i++) {
    file.print(history[i].timeStr);
    file.print(",");
    file.print(history[i].temperature, 2);
    file.print(",");
    file.print(history[i].humidity, 2);
    file.print(",");
    file.println(history[i].pressure, 2);
  }

  file.close();
  Serial.println("✅ Історія збережена у CSV: " + path);
}

*/


/*
void saveHistoryToFile() {
  String filename = String("/log/") + getTodayDate() + ".csv";
  File f = SPIFFS.open(filename, FILE_WRITE);
  if (!f) {
    Serial.println("[ERR] Не вдалося створити лог-файл");
    return;
  }
  f.println("Час,Температура,Вологість,Тиск");
  for (int i = 0; i < MAX_POINTS; i++) {  // 🔄 було MAX_MEASUREMENTS
    int idx = (historyIndex + i) % MAX_POINTS;
    BMEData& d = history[idx];
    f.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
  }
  f.close();
  Serial.println("[OK] Збережено: " + filename);
}
*/



// --- Видалити лог-файли старші за N днів ---
void clearOldLogs(int maxDays) {
  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) return;

  time_t now;
  time(&now);

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    file.close();

    if (name.endsWith(".csv")) {
      name.replace("/log/", "");
      struct tm tm = {};
      if (strptime(name.c_str(), "%Y-%m-%d.csv", &tm)) {
        time_t fileTime = mktime(&tm);
        double ageDays = difftime(now, fileTime) / 86400.0;
        if (ageDays > maxDays) {
          String fullpath = String("/log/") + name;
          SPIFFS.remove(fullpath);
          Serial.println("[DEL] Старий файл: " + fullpath);
        }
      }
    }
    file = root.openNextFile();
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
  WiFi.begin("Smart_House", "Telemat5311051");
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
    //request->send(404, "text/plain", "❌ CSV файл не знайдено");
    request->send(404, "text/html; charset=utf-8", "<h2>❌ Файл не знайдено</h2>");

    return;
  }

  request->send(SPIFFS, path, "text/csv", true);  // true = файл завантажується
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


/*
server.on("/bme_data", HTTP_GET, [](AsyncWebServerRequest *request){
  float temperature = bme.readTemperature();    // Температура в градусах Цельсія
  float humidity = bme.readHumidity();          // Вологість у %
  float pressure = bme.readPressure() / 100.0F; // Тиск у гектопаскалях (hPa)
  float presmmhg = bme.readPressure() / 133.322F; // Тиск у mmHg (додайте F для float-літералу)

  // Створюємо JSON-рядок з даними
  String jsonResponse = "{";
  jsonResponse += "\"temperature\": " + String(temperature, 2) + ","; // 2 знаки після коми
  jsonResponse += "\"humidity\": " + String(humidity, 2) + ",";
  jsonResponse += "\"pressure\": " + String(pressure, 2) + ",";       // <<< ДОДАНО КОМУ ТУТ
  jsonResponse += "\"presmmhg\": " + String(presmmhg, 2);            // <<< ВИКОРИСТАННЯ presmmhg
  jsonResponse += "}";

  request->send(200, "application/json", jsonResponse);
});
*/

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



//+++++++++++Працююча версія для графіків без динамічних вісей
/*
server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print("[");
  bool first = true;

  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr.length() == 0) continue;  // Пропустити порожні записи

    if (!first) response->print(",");
    first = false;

    response->printf(
      "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
      d.timeStr.c_str(), d.temperature, d.humidity, d.pressure
    );
  }

  response->print("]");
  request->send(response);
});

*/

/*
server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->print("[");
  bool first = true;

  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;  // пропустити порожні записи

    if (!first) response->print(",");  // кома між об'єктами
    first = false;

    response->printf(
      "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
      d.timeStr.c_str(), d.temperature, d.humidity, d.pressure
    );
  }

  response->print("]");
  request->send(response);
});
*/
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
/*
server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");
  request->send(response);
});
*/
server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
  // Більш правильний і лаконічний спосіб:
  // Вказати повний Content-Type разом з charset одразу у send()
  request->send(SPIFFS, "/log_viewer.html", "text/html; charset=utf-8");
});

//===============================================================================

/*
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
    name.replace("/log/", "");         // 2025-06-27.csv
    name.replace(".csv", "");          // 2025-06-27

    html += "<li>";
    html += "<a href=\"/log_viewer?date=" + name + "\">📊 " + name + "</a> ";
    html += "(<a href=\"" + path + "\">csv</a>)";
    html += "</li>";

    file = root.openNextFile();
  }

  html += "</ul><p><a href=\"/\">На головну</a></p>";
  request->send(200, "text/html", html);
});
*/
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
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
 saveHistoryToFile();
  request->send(200, "text/plain", "✅ Поточний архів збережено у файл.");
});
//++++++++++++++++++++++++++
//"text/html; charset=utf-8"
  //server.onNotFound([](AsyncWebServerRequest *request){
    //request->send(404, "text/plain", "Сторінка не знайдена!"); //"text/html; charset=utf-8"
 // });

server.onNotFound([](AsyncWebServerRequest *request){
  // Встановлюємо Content-Type як HTML і вказуємо кодування UTF-8
  request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
});

static String lastSavedDate;
String today = getTodayDate();
if (today != lastSavedDate) {
  saveHistoryToFile();
  clearOldLogs(30);
  lastSavedDate = today;
}
/*
// Обробник для файлів у папці /log (архіви CSV)
  server.onRegex("^/log/[^/]+\\.csv$", HTTP_GET, [](AsyncWebServerRequest *request){
    String path = request->url(); // Отримуємо повний шлях, наприклад, "/log/2025-06-30.csv"
    if (SPIFFS.exists(path)) {
      // Важливо: встановлюємо Content-Type як text/csv та вказуємо charset=utf-8
      request->send(SPIFFS, path, "text/csv; charset=utf-8");
    } else {
      request->send(404, "text/plain", "Файл архіву не знайдено!");
    }
  });
*/
  server.begin();
}

void loop() {
  
  //======================================
    static uint32_t lastUpdate = 0;
  
  if (millis() - lastUpdate > 60000) {  //оновлення даних для графіків в мілісекундах
    lastUpdate = millis();
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;
///////////////////Побудова графіків
float t = bme.readTemperature();
float h = bme.readHumidity();
float p = bme.readPressure() / 100.0;
//String timeStr = getTimeStr(); // ваша функція або rtc
String timeStr = getCurrentTime();
// У вашому циклі оновлення даних:
// ...
//String timeStr = getCurrentTime();
Serial.printf("DEBUG: getCurrentTime() повернуло: '%s'\n", timeStr.c_str()); // ДОДАЙТЕ ЦЕЙ РЯДОК
storeToHistory(t, h, p, timeStr);
// ...
storeToHistory(t, h, p, timeStr);

 if (historyIndex >= 10 && getTodayDate() != lastSavedDate) {
  saveHistoryToFile();
  clearOldLogs(30);
  lastSavedDate = getTodayDate();
////////////////
  lastTemp = bme.readTemperature();
  lastHum = bme.readHumidity();
  lastPress = bme.readPressure() / 100.0F;
///////////////
  }
  delay(10000);

//=========================================================

  //Оригінальний код
  tft.setRotation(1);//Альбомная орієнтація
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);//Очистить дисплей
//////////////////
 
tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону 

 //server.handleClient();  // обробка запитів 
 



///////////////////////
   // Виведення зображення з масиву
/*
 Змінюємо розмір зображення jpeg до необхідного https://imageconvert.org/image-resizer
 Конвертуємо зображення в масив .с https://lvgl.io/tools/imageconverter
 Створюємо в корені проєкту окрему технічну папку, напр. tools/lvgl_sources/, і 
 копіюємо туди .c-файли масиву з LVGL.
 В корінь проєкту копіюємо скрипт convert_lvgl_image.py
 Налаштовуємо в скрипті convert_lvgl_image.py шляхи та назви файлів
 У Windows 10 в PowerShell переходимо до папки з файлом convert_lvgl_image.py і запускаємо
 скрипт командою python convert_lvgl_image.py (відкрити в провіднику папку проекту, куди скопійовано скрипт,
 скопіювати шлях до папки у вікні провідника, відкрити PowerShell, ввести:
  cd "C:\Users\eatam\OneDrive\Documents\PlatformIO\Projects\Weather_station" python convert_lvgl_image.py)
 Якщо все правильно, то в папці з проєктом з'явиться файл image_My_image.h (імя файлу налаштовується в скрипті)
 В кінці масива файл image_My_image.h закоментовуємо два рядка //#define IMAGE_WIDTH 159
                                                               //#define IMAGE_HEIGHT 119
Включаємо в проєкт файл image_My_image.h
 Командою tft.pushImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, image_My_image.h); (x,y,ширина,висота,масив зображення)
 виводимо зображення на екран. Ширина і висота в команді задаються в явному вигляді. 
*/
  //tft.pushImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, image_Tolya);
  //tft.pushImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, image_140_105_Tolya);
  //tft.pushImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, image_164_124_Tolya);
  //tft.pushImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, image_153_115_Tolya);

   //tft.pushImage(15, 10, IMAGE_WIDTH, IMAGE_HEIGHT, image_140_105_Tolya);
    //tft.pushImage(4, 6, 153, 115, image_153_115_Tolya);
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

     
if (switch1)
      {
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
    
      //tft.fillScreen(TFT_BLACK);//Очистить дисплей

      tft.setCursor(80, 30);//Поставити курсор Х У
      tft.setTextColor(TFT_GREEN, TFT_BLACK);//Колір шрифта, колір фону  
      
      if(hour<10)
      {
      tft.print(0);
      tft.print(hour);} 
      else
      {
      tft.print(hour);
      }

      tft.drawString(":", 110, 30);//Встановити курсор в позицію Х У
  
      tft.setCursor(120, 30);
      if(minute<10)
      {
      tft.print(0);
      tft.print(minute); } 
      else
      {tft.print(minute);
      }

      tft.setCursor(40, 80);//Встановити курсор в позицію Х У
      
      if(day<10)
      {
      tft.print(0);
      tft.print(day); } 
      else
      {tft.print(day);
      } 
      tft.drawString(":",75, 80);//Встановити курсор в позицію Х У
      tft.setCursor(85, 80);//Поставити курсор Х У
      if(month<10)
      {
      tft.print(0);
      tft.print(month); } 
      else
      {tft.print(month);
        }   
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

      
      if (switch2)
      //if(0)
      {
          
 
      i=10;
      x=0;
      //Усереднюємо значення тиску
   
      while(i)
      {
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
 
}  



