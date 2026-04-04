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