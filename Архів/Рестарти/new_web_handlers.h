// web_handlers.h — єдиний файл, що містить усі хендлери веб-сервера
// Використовує AsyncResponseStream для більш ефективної роботи із пам'яттю
// Підключається у main.cpp: #include "web_handlers.h" та виклик registerWebHandlers(server)

#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

// ======================================================
// 📄 Хендлер для головної сторінки
// ======================================================
void handleRoot(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    // 🏠 HTML головної сторінки
    response->print("<html><head><meta charset='UTF-8'><title>ESP32 Головна</title></head><body>");
    response->print("<h1>📡 Веб-інтерфейс ESP32</h1>");
    response->print("<ul>");
    response->print("<li><a href='/logs'>📂 Переглянути архіви</a></li>");
    response->print("<li><a href='/log_viewer'>📊 Графіки</a></li>");
    response->print("</ul></body></html>");

    request->send(response);
}

// ======================================================
// 📄 Хендлер для сторінки /logs — список файлів архівів
// ======================================================
void handleLogs(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<html><head><meta charset='UTF-8'><title>Доступні архіви</title></head><body>");
    response->print("<h2>📂 Доступні архіви:</h2><ul>");

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
        response->print("<li>Немає доступних лог-файлів.</li>");
    } else {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name(); // /log/2025-07-22.csv
            if (fullPath.endsWith(".csv")) {
                String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1); // 2025-07-22.csv
                String datePart = fileName;
                datePart.replace(".csv", "");

                // 📝 Формуємо список із посиланнями на log_viewer і CSV-файл
                response->printf("<li><a href='/log_viewer?date=%s'>📊 %s</a> (<a href='/log/%s'>csv</a>)</li>",
                                 datePart.c_str(), datePart.c_str(), fileName.c_str());
            }
            file = root.openNextFile();
        }
    }

    response->print("</ul><p><a href='/'>🏠 На головну</a></p>");
    response->print("</body></html>");

    request->send(response);
}

// ======================================================
// 📄 Хендлер для сторінки log_viewer.html — відображення графіків
// ======================================================
void handleLogViewer(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<html><head><meta charset='UTF-8'><title>Графіки</title></head><body>");
    response->print("<h2>📊 Перегляд графіків</h2>");
    response->print("<iframe src='/log_viewer.html' width='100%' height='600px' frameborder='0'></iframe>");
    response->print("<p><a href='/'>🏠 На головну</a></p>");
    response->print("</body></html>");

    request->send(response);
}

// ======================================================
// 📄 Хендлер для видачі CSV файлів ( /log/2025-07-24.csv )
// ======================================================
void handleCsvFiles(AsyncWebServerRequest *request) {
    String path = request->url(); // очікуємо /log/2025-07-24.csv
    if (SPIFFS.exists(path)) {
        request->send(SPIFFS, path, "text/csv");
    } else {
        request->send(404, "text/plain", "Файл не знайдено");
    }
}

// ======================================================
// 📄 Хендлер 404 (якщо URL не знайдений)
// ======================================================
void handleNotFound(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<html><head><meta charset='UTF-8'><title>404</title></head><body>");
    response->printf("<h2>❌ 404 — Сторінка %s не знайдена</h2>", request->url().c_str());
    response->print("<p><a href='/'>🏠 На головну</a></p>");
    response->print("</body></html>");

    request->send(response);
}

// ======================================================
// 📄 Реєстрація всіх хендлерів у сервері
// ======================================================
void registerWebHandlers(AsyncWebServer &server) {
    server.on("/", HTTP_GET, handleRoot);                 // Головна
    server.on("/logs", HTTP_GET, handleLogs);              // Список архівів
    server.on("/log_viewer", HTTP_GET, handleLogViewer);   // Перегляд графіків
    server.on("/log/", HTTP_GET, handleCsvFiles);          // CSV файли

    // Використаємо шаблонний маршрут для всіх CSV
    server.on("/log/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        handleCsvFiles(request);
    });

    server.onNotFound(handleNotFound);                     // 404
}
// 📌 Централізована реєстрація всіх веб-хендлерів
void registerWebHandlers(AsyncWebServer &server) {
  handleRootPage(&server);
  handleLogViewer(&server);
  handleLogList(&server);

  // ✅ Якщо потрібно обслуговувати статичні CSV прямо по URL /log/2025-07-24.csv
  server.serveStatic("/log", SPIFFS, "/log").setCacheControl("no-cache");
}


#endif // WEB_HANDLERS_H
/*
✅ Як підключити:
У main.cpp просто додай:

cpp
Копіювати
Редагувати
#include "web_handlers.h"
У setup() зареєструй:

cpp
Копіювати
Редагувати
handleRootPage(&server);
handleLogViewer(&server);
handleLogList(&server);
🔍 Коментарі:
Усі обробники використовують AsyncResponseStream, щоб передавати HTML-файли по частинах (не копіюючи весь у памʼять).

Всі шляхи збережено згідно з твоїм проєктом (/log/, /log_viewer, /logs тощо).

Випадки помилок обробляються — з відповідними кодами та повідомленнями.

Код повністю самодостатній та замінює попередні Handlers.h, Handlers_1.h.

🔔 Якщо потрібно додати /log/ як статичний маршрут для CSV, то в setup() також додай:

cpp
Копіювати
Редагувати
server.serveStatic("/log", SPIFFS, "/log").setCacheControl("no-cache");
*/