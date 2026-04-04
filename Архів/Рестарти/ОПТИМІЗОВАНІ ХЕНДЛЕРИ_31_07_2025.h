// ===================== 🌐 ОПТИМІЗОВАНІ ХЕНДЛЕРИ =====================

// 📄 Головна сторінка (index.html)
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/] Головна сторінка");
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
});

// 📄 Сторінка налаштувань
server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/settings]");
  request->send(SPIFFS, "/settings.html", "text/html", false, processor);
});

// 📄 Графік
server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/graph.js]");
  request->send(SPIFFS, "/graph.js", "application/javascript");
});

// ===================== 📊 BME-280 JSON та архіви =====================

// ✅ Дані з BME280 у JSON
server.on("/bme_data", HTTP_GET, [](AsyncWebServerRequest *request) {
  Serial.printf("🧠 Heap before /bme_data: %u bytes\n", ESP.getFreeHeap());
  
  DynamicJsonDocument doc(256);
  doc["temperature"] = bme.readTemperature();
  doc["humidity"]    = bme.readHumidity();
  doc["pressure"]    = bme.readPressure() / 100.0F;
  doc["presmmhg"]    = bme.readPressure() / 133.322F;

  String output;
  serializeJson(doc, output);
  request->send(200, "application/json", output);
});

// ✅ Графік поточних даних
server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
  Serial.printf("🧠 Heap before /bme_chart_data: %u bytes\n", ESP.getFreeHeap());

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

// ✅ Графік із архіву
server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
  Serial.printf("🧠 Heap before /bme_chart_data_archive: %u bytes\n", ESP.getFreeHeap());

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









// ===================== 📁 СПИСОК ЛОГІВ =====================

server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
  Serial.printf("🧠 Heap before /logs: %u bytes\n", ESP.getFreeHeap());

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
});

// ===================== 📄 ПРОГЛЯДАЧ ЛОГІВ =====================

server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/log_viewer]");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");
  request->send(response);
});

// ===================== 🛠 ІНШІ =====================

// 🔁 Перезапуск
server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
  Serial.println("🔁 [ESP RESTART]");
  request->send(200, "text/html", "<h2>Пристрій перезавантажується...</h2>");
  delay(500);
  ESP.restart();
});




















// ✅ Подаємо статичні файли
server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

// ✅ Обробка 404
server.onNotFound([](AsyncWebServerRequest *request){
  request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
});
