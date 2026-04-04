server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/] Головна сторінка");
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
});