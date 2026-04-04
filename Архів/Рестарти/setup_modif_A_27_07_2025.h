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
 WiFi.begin("Smart_House", "Telemat5311051");
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
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  }

  // =====================================================================================
  // 🌍 📡 ВЕБ-СЕРВЕР: ГОЛОВНІ HTML-СТОРІНКИ
  // =====================================================================================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });

  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });

  // =====================================================================================
  // 📊 📈 **JSON API для графіків**
  // =====================================================================================

  // 🔵 1. Поточні дані з BME280 (графік реального часу)
  server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
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
  });

  // 🔵 2. Архівні дані (по конкретній даті)
  server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("date")) {
      request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
      return;
    }

    String date = request->getParam("date")->value();
    String path = "/log/" + date + ".csv";
    if (!SPIFFS.exists(path)) {
      request->send(404, "application/json", "[]");
      return;
    }

    File file = SPIFFS.open(path, FILE_READ);
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

  // === Перший екран на TFT ===
  currentDisplayMode = MODE_DATETIME;
  executeDisplayMode(currentDisplayMode);
  displayModeStartTime = millis();
}
