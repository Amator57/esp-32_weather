server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request){
  Serial.println("➡️ [/graph.js]");
  request->send(SPIFFS, "/graph.js", "application/javascript");
});