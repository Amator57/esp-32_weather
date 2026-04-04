server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/settings]");
request->send(SPIFFS, "/settings.html", "text/html", false, processor);
});