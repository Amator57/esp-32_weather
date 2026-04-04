#include <ArduinoJson.h>  // Не забудь підключити

server.on("/archive_chart_data", HTTP_GET, [](AsyncWebServerRequest *request){
  const size_t capacity = JSON_ARRAY_SIZE(archiveCount) + archiveCount * JSON_OBJECT_SIZE(4) + 1024;
  DynamicJsonDocument doc(capacity);

  for (size_t i = 0; i < archiveCount; i++) {
    JsonObject entry = doc.createNestedObject();
    entry["time"] = archiveData[i].time;
    entry["temperature"] = archiveData[i].temperature;
    entry["humidity"] = archiveData[i].humidity;
    entry["pressure"] = archiveData[i].pressure;
  }

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
});

server.on("/archive_chart_data", HTTP_GET, [](AsyncWebServerRequest *request){
  if (SPIFFS.exists("/archive.json")) {
    request->send(SPIFFS, "/archive.json", "application/json");
  } else {
    request->send(404, "text/plain", "Архів не знайдено");
  }
});
