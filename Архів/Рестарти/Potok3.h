server.on("/graph.js", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(SPIFFS, "/graph.js", "application/javascript");
});