server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
request->send(SPIFFS, "/settings.html", "text/html", false, processor);
});