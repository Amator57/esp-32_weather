server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
     Serial.printf("🧠 Free Heap before /logs: %u bytes\n", ESP.getFreeHeap());// Додав 30.07.2025
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");
    response->print(F("<h2>Доступні архіви:</h2><ul>"));
 
    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
      response->print(F("<li>Немає доступних лог-файлів.</li>"));
    } else {
      File file = root.openNextFile();
      while (file) {
        String fullPath = file.name();
        if (fullPath.endsWith(".csv")) {
          String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
          String datePart = fileName;
          datePart.replace(".csv", "");

          response->print("<li>");
          response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
          response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
          response->print("</li>");
        }
        file = root.openNextFile();
      }
    }
    response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
  });