void setup() {
  // === Ініціалізація серіалу і дисплея ===
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  // === RTC, I2C, SPIFFS ===
  Wire.begin();
  rtc.begin();

  Serial.println("Ініціалізація SPIFFS...");
  if (!SPIFFS.begin()) {
    Serial.println("❌ Помилка при монтуванні SPIFFS");
    while (true) { delay(100); } // Зупинка, якщо SPIFFS не монтується
  }
  Serial.println("✅ SPIFFS успішно змонтовано.");

  resetHistoryForNewDay();
  Serial.println("✅ Індекс історії успішно очищено.");
  //=========================
  //Видалення конкретного лог-файлу
       // deleteLogFile("unknown.csv"); //  deleteLogFile("2025-07-10.csv");
       // Serial.println("✅ Заданий файл історії успішно очищено.");
  //=========================
  // === Створення директорії /log якщо відсутня ===
  if (!SPIFFS.exists("/log")) {
    Serial.println("[SETUP] Директорія /log не знайдена, створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити директорію /log!");
      while (true) { delay(100); }
    }
    Serial.println("✅ /log успішно створено.");
  } else {
    Serial.println("[SETUP] Директорія /log вже існує.");
  }

  // === Перевірка вмісту SPIFFS ===
  Serial.println("Перевіряю вміст SPIFFS:");
  File root = SPIFFS.open("/");
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      Serial.printf(" FILE: %s (%d bytes)\n", file.name(), file.size());
      file = root.openNextFile();
    }
  }
  Serial.println("====================================");

  // === Ініціалізація BME280 ===
  bme.begin(0x76);
  for (int j = 0; j < MAX_MEASUREMENTS; ++j) {
      history[j].timeStr = "";
      history[j].temperature = 0.0f;
      history[j].humidity = 0.0f;
      history[j].pressure = 0.0f;
  }

  // === Завантаження конфігурації ===
  loadConfig();
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();

  // === Wi-Fi: спроба STA, fallback на AP ===
  WiFi.begin("Smart_House", "Telemat5311051");
  delay(3000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ STA не підключився, запускаю AP...");
    if (WiFi.softAP("ESP32_Config", "12345678")) {
      String apIP = WiFi.softAPIP().toString();
      Serial.println("✅ AP запущено. IP: " + apIP);
      tft.drawString("AP Mode: " + apIP, 10, 50);
    } else {
      Serial.println("❌ Не вдалося запустити AP!");
      tft.drawString("AP Mode Failed!", 10, 50);
    }
  } else {
    wifiSSID = WiFi.SSID();
    String ip = WiFi.localIP().toString();
    Serial.println("✅ WiFi: " + wifiSSID + ", IP: " + ip);
    tft.drawString("WiFi: " + wifiSSID, 10, 50);
    tft.drawString("IP: " + ip, 10, 70);
  }

  delay(3000);

  // === Перше зчитування BME280 після запуску ===
  lastTemp = bme.readTemperature();
  lastHum = bme.readHumidity();
  lastPress = bme.readPressure();
  Serial.printf("Initial BME280: %.1f°C, %.1f%%, %.1fPa\n", lastTemp, lastHum, lastPress);

  // === NTP синхронізація ===
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  }

  // ======================================================================================
  // 📌 📌 📌 ВЕБ-СЕРВЕР 📌 📌 📌
  // ======================================================================================

  // 🏠 ГОЛОВНА СТОРІНКА
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });

  // 📄 JS для графіка
  server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/graph.js", "application/javascript");
  });

  // ⚙️ СТОРІНКА НАЛАШТУВАНЬ
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });

  // 🔄 Перезавантаження ESP32
  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<h2>🔁 ESP32 перезавантажується...</h2>");
    delay(500);
    ESP.restart();
  });

  // 🌡️ BME Дані JSON (поточні)
  server.on("/bme_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(256);
    doc["temperature"] = bme.readTemperature();
    doc["humidity"]    = bme.readHumidity();
    doc["pressure"]    = bme.readPressure() / 100.0F;
    doc["presmmhg"]    = bme.readPressure() / 133.322F;
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // 📊 Список логів (HTML)
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
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

  // 🗂 Сторінка перегляду логів
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });

  // 💾 Збереження налаштувань
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("device_name", true))
      deviceName = request->getParam("device_name", true)->value();
    if (request->hasParam("ssid", true))
      wifiSSID = request->getParam("ssid", true)->value();
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

    saveConfig();
    request->send(200, "text/html; charset=utf-8",
      "<script>alert('Налаштування збережено!');location.href='/settings';</script>");
  });

  // === Обслуговування статичних файлів та логів ===
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/log/", SPIFFS, "/log/").setCacheControl("no-store");

  // === Обробка 404 ===
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
  });

  // ======================================================================================
  // 🚀 Запуск сервера
  // ======================================================================================
  server.begin();

  // === Перший екран на дисплеї ===
  currentDisplayMode = MODE_DATETIME;
  executeDisplayMode(currentDisplayMode);
  displayModeStartTime = millis();
}
