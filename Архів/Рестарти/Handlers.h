//Працює. Можлива причина рестартів
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    request->send(response);
  });
 ////////////////////////
// Працює. Можлива причина рестартів
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
  bool *param = new bool(false);  // ❗ ручне збереження без очищення масиву
  xTaskCreatePinnedToCore(
    saveHistoryTask,
    "SaveHistory",
    8192,
    param,
    1,
    NULL,
    1
  );
  request->send(200, "text/plain", "✅ Ручне збереження розпочато у фоновому режимі.");
});
////////////////////////////////////
// Працює. Заміна - намагання уникнути рестартів
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");

  response->print(F("<h2>Доступні архіви:</h2><ul>"));

  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    response->print(F("<li>Немає доступних лог-файлів.</li>"));
  } else {
    File file = root.openNextFile();
    while (file) {
      String fullPath = file.name();  // Наприклад: /log/2025-07-22.csv

      if (!fullPath.endsWith(".csv")) {
        file = root.openNextFile();
        continue;
      }

      String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);  // 2025-07-22.csv
      String datePart = fileName;
      datePart.replace(".csv", "");  // 2025-07-22

      response->print("<li>");
      response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
      response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
      response->print("</li>");

      file = root.openNextFile();
    }
  }

  response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
  request->send(response);
});
////////////////////////////////////////////////////////
//Працює. Можливо причина рестартів
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });