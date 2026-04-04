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