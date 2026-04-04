server.onNotFound([](AsyncWebServerRequest *request){
  request->send(404, "text/html; charset=utf-8", "<h1>404</h1><p>Сторінка не знайдена!</p>");
});