#pragma once
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// ✅ Хендлер головної сторінки (index.html)
void handleRoot(AsyncWebServerRequest *request) {
  // Відправляємо index.html з SPIFFS із підтримкою шаблонів (processor)
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

// ✅ Хендлер списку логів (HTML-сторінка)
void handleLogs(AsyncWebServerRequest *request) {
  // Використовуємо потік, щоб уникнути створення великого HTML у RAM
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");

  // Заголовок сторінки
  response->println(F("<h2>Доступні архіви:</h2><ul>"));

  // Відкриваємо директорію /log
  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    // Якщо немає директорії або вона не є директорією
    response->println(F("<li>Немає доступних лог-файлів.</li>"));
  } else {
    // Перебираємо всі файли в /log
    File file = root.openNextFile();
    while (file) {
      String fullPath = file.name();   // /log/2025-07-22.csv

      // Пропускаємо файли, які не закінчуються на .csv
      if (!fullPath.endsWith(".csv")) {
        file = root.openNextFile();
        continue;
      }

      // Отримуємо тільки ім'я файлу (2025-07-22.csv)
      String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
      String datePart = fileName;
      datePart.replace(".csv", "");  // 2025-07-22

      // Формуємо HTML-рядок для кожного логу
      response->print("<li>");
      response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
      response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
      response->println("</li>");

      file = root.openNextFile();
    }
  }

  // Закриваємо список
  response->println(F("</ul><p><a href=\"/\">На головну</a></p>"));

  // Відправляємо відповідь
  request->send(response);
}

// ✅ Хендлер перегляду графіка (log_viewer.html)
void handleLogViewer(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/log_viewer.html", "text/html");
  response->addHeader("Content-Type", "text/html; charset=utf-8");
  request->send(response);
}

/*
📄 Як підключити в main.cpp

#include "web_handlers.h"

// У setup():
server.on("/", HTTP_GET, handleRoot);
server.on("/logs", HTTP_GET, handleLogs);
server.on("/log_viewer", HTTP_GET, handleLogViewer);
*/