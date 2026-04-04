server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/log_viewer]");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");
  request->send(response);
});