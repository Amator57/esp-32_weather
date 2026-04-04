// ===================== 🌐 ОПТИМІЗОВАНІ ХЕНДЛЕРИ =====================


 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  
  
  
 server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html", false, processor);
  });  
 
 
  // 📄 JS для графіка
  server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/graph.js", "application/javascript");
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
  
  
  
  
  //Замінено 29.07.2025. Функція на тестуванні
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
											 
			  
    if (!file) {
        request->send(500, "application/json", "[]");
        return;													 												 
    }

    Serial.printf("📈 [ARCHIVE] Sending %s (size: %u bytes)\n", path.c_str(), file.size());
    Serial.printf("📈 [ARCHIVE] Free heap before: %u bytes\n", ESP.getFreeHeap());

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");
    bool first = true;

    int lineCounter = 0;
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
        lineCounter++;
        if (lineCounter > 500) {  // ⚠️ обмеження – не більше 500 точок
            Serial.println("⚠️ [ARCHIVE] JSON обрізано до 500 рядків!");
            break;
        }
    }
    file.close();

    response->print("]");
																									   																	
    request->send(response);
	 
    Serial.printf("📈 [ARCHIVE] JSON sent (%d lines). Free heap after: %u bytes\n", lineCounter, ESP.getFreeHeap());
 });
 
 
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
  
  
  
  
  
  
  
  
  
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });
  
  
  
  
  
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
    request->send(SPIFFS, "/graph.js", "application/javascript");
  });
  
 server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); 
 
 
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
  });