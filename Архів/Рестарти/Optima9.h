server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
  Serial.println("🔁 [ESP RESTART]");
  request->send(200, "text/html", "<h2>Пристрій перезавантажується...</h2>");
  delay(500);
  ESP.restart();
});