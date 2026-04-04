//Використовувалася з  web_handlers.h, але результата поганий


void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);      // Задати альбомну орієнтацію один раз
  tft.setTextWrap(false);  // Задати обгортання тексту один раз
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  Wire.begin();
  rtc.begin();

  //+++++++++++++++++++++++++++++++++
  Serial.println("Ініціалізація SPIFFS...");
  if (!SPIFFS.begin()) {
    Serial.println("❌ Помилка при монтуванні SPIFFS");
    while (true) { delay(100); } // Застрягти тут, якщо SPIFFS не монтується
  }
  Serial.println("✅ SPIFFS успішно змонтовано.");

  resetHistoryForNewDay(); // скидаємо індекс історії на нуль
  Serial.println("✅ Індекс історії успішно очищено.");

  //=========================
  // Видалення конкретного лог-файлу (опційно)
  // deleteLogFile("2025-07-10.csv");
  //=========================

  // ==== Код для створення директорії /log ====
  if (!SPIFFS.exists("/log")) {
    Serial.println("[SETUP] Директорія /log не знайдена, створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ [ERROR] Не вдалося створити директорію /log!");
      while (true) { delay(100); }
    }
    Serial.println("✅ [SETUP] Директорія /log успішно створена.");
  } else {
    Serial.println("[SETUP] Директорія /log вже існує.");
  }
  // =========================================================

  // ==== Діагностика вмісту SPIFFS (залишено) ====
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

  //+++++++++++++++++++++++++++++++++
  bme.begin(0x76);

  // ІНІЦІАЛІЗАЦІЯ МАСИВУ ІСТОРІЇ
  for (int j = 0; j < MAX_MEASUREMENTS; ++j) {
      history[j].timeStr = "";
      history[j].temperature = 0.0f;
      history[j].humidity = 0.0f;
      history[j].pressure = 0.0f;
  }

  loadConfig();

  // --- TZ і WiFi ---
  String tzString = buildTZString(tzOffset, useDST);
  setenv("TZ", tzString.c_str(), 1);
  tzset();

 WiFi.begin("Smart_House", "Telemat5311051");
  delay(3000); // Час на підключення

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Режим STA не підключився, запускаю режим AP...");
    if (WiFi.softAP("ESP32_Config", "12345678")) {
      String apIP = WiFi.softAPIP().toString();
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

  // === Перше зчитування даних BME280 ===
  lastTemp = bme.readTemperature();
  lastHum = bme.readHumidity();
  lastPress = bme.readPressure();
  Serial.printf("Initial BME280: Temp=%.1fC, Hum=%.1f%%, Press=%.1fPa\n", lastTemp, lastHum, lastPress);

  // --- ПІСЛЯ інтеграції web_handlers.h ---
  registerWebHandlers(server);
server.begin();
Serial.println("✅ Web Server запущено!");
  //+++++++++++++++++++++++++++++++++
  // ⛔ Старі дублікати (інші виклики server.on, які тепер винесено у web_handlers.h)
  // ⛔ ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
  /*
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Цей код дублює handleRootPage() у web_handlers.h
    });

    server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Цей код дублює handleLogList() у web_handlers.h
    });

    server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Цей код дублює handleLogViewer() у web_handlers.h
    });
  */
  // ⛔ ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
}