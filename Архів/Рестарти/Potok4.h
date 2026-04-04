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