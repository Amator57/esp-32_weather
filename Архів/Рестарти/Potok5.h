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