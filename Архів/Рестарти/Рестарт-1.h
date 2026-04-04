

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Web</title></head><body>");
    response->print("<h1>Вітаю на ESP32 Web Server!</h1>");
    response->print("<ul>");
    response->print("<li><a href=\"/logs\">📂 Переглянути архіви</a></li>");
    response->print("<li><a href=\"/log_viewer\">📊 Переглянути графіки</a></li>");
    response->print("<li><a href=\"/save_now\">💾 Зберегти архів зараз</a></li>");
    response->print("</ul>");
    response->print("</body></html>");

    request->send(response);
});

////////////
server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<h2>Доступні архіви:</h2><ul>");

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
        response->print("<li>Немає доступних лог-файлів.</li>");
    } else {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name();   // /log/2025-07-24.csv
            if (fullPath.endsWith(".csv")) {
                String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1); // 2025-07-24.csv
                String datePart = fileName;
                datePart.replace(".csv", "");

                response->printf("<li><a href=\"/log_viewer?date=%s\">📊 %s</a> (<a href=\"%s\">csv</a>)</li>",
                                 datePart.c_str(), datePart.c_str(), fullPath.c_str());
            }
            file = root.openNextFile();
        }
    }

    response->print("</ul><p><a href=\"/\">На головну</a></p>");
    request->send(response);
});
//////////////////////////////////////////////
server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    response->print("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    response->print("<title>Перегляд графіків</title>");
    response->print("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>");
    response->print("</head><body>");
    response->print("<h2>📊 Графіки за датою</h2>");
    response->print("<canvas id='chart' width='400' height='200'></canvas>");
    response->print("<p><a href='/'>На головну</a></p>");
    response->print("<script>");
    response->print("// Тут можна додати JS код для підвантаження CSV та побудови графіку");
    response->print("</script>");
    response->print("</body></html>");

    request->send(response);
});
////////////////////////////////////
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
    bool *param = new bool(false);  // ❌ не очищати history після ручного збереження
    xTaskCreatePinnedToCore(
        saveHistoryTask,
        "SaveHistory",
        8192,
        param,
        1,
        NULL,
        1
    );
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    response->addHeader("Content-Type", "text/plain; charset=utf-8");
    response->print("✅ Ручне збереження розпочато у фоновому режимі.");
    request->send(response);
});
/////////////////////////////////
📌 Що це дає:
✅ Всі сторінки відправляються потоково, RAM більше не переповнюється.
✅ Великі CSV, довгі списки архівів або HTML не призводять до виклику cbuf::resizeAdd() і падінь ESP32.

👉 Хочеш, щоб я:

🔄 Вставив ці хендлери прямо в твій main.cpp замість існуючих?

📄 Додав коментарі до кожного блоку (щоб було видно, чому AsyncResponseStream краще)?

⚡ Або зробив окремий файл web_handlers.h, щоб код був акуратнішим?



#pragma once
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// ================================================================
// 🌐 1. ГОЛОВНА СТОРІНКА /
// ================================================================
void handleRoot(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
    
    response->print(F("<html><head><meta charset='UTF-8'><title>ESP32 Web Server</title></head><body>"));
    response->print(F("<h1>📊 ESP32 Метеостанція</h1>"));
    response->print(F("<p><a href='/logs'>📂 Переглянути архіви</a></p>"));
    response->print(F("<p><a href='/log_viewer'>📈 Переглянути графік</a></p>"));
    response->print(F("<p><a href='/save_now'>💾 Зберегти архів зараз</a></p>"));
    response->print(F("</body></html>"));

    request->send(response);
}

// ================================================================
// 🌐 2. СПИСОК ЛОГІВ /logs
// ================================================================
void handleLogs(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");

    response->print(F("<h2>Доступні архіви:</h2><ul>"));

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
        response->print(F("<li>Немає доступних лог-файлів.</li>"));
    } else {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name();
            if (!fullPath.endsWith(".csv")) {
                file = root.openNextFile();
                continue;
            }

            String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
            String datePart = fileName;
            datePart.replace(".csv", "");

            response->print(F("<li>"));
            response->printf("<a href='/log_viewer?date=%s'>📊 %s</a> ", datePart.c_str(), datePart.c_str());
            response->printf("(<a href='/log/%s'>csv</a>)", fileName.c_str());
            response->print(F("</li>"));

            file = root.openNextFile();
        }
    }

    response->print(F("</ul><p><a href='/'>На головну</a></p>"));

    request->send(response);
}

// ================================================================
// 🌐 3. ПЕРЕГЛЯД ГРАФІКІВ /log_viewer
// ================================================================
void handleLogViewer(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");

    response->print(F("<html><head><meta charset='UTF-8'><title>Перегляд графіків</title>"));
    response->print(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>"));
    response->print(F("<h2>📈 Перегляд даних</h2>"));

    if (request->hasParam("date")) {
        String date = request->getParam("date")->value();
        response->printf("<h3>Дата: %s</h3>", date.c_str());
        response->printf("<iframe src='/log/%s.csv' width='100%%' height='400px'></iframe>", date.c_str());
    } else {
        response->print(F("<p>❌ Не вибрано дату для перегляду.</p>"));
    }

    response->print(F("<p><a href='/logs'>⬅️ Назад до архівів</a></p></body></html>"));

    request->send(response);
}

// ================================================================
// 🌐 4. РУЧНЕ ЗБЕРЕЖЕННЯ /save_now
// ================================================================
extern void saveHistoryTask(void *parameter); // оголошення задачі (щоб уникнути помилок)

void handleSaveNow(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain; charset=utf-8");

    response->print(F("✅ Ручне збереження розпочато у фоновому режимі.\n"));

    bool *param = new bool(false);  // ❌ без очищення history
    xTaskCreate(
        saveHistoryTask,
        "SaveHistory",
        8192,
        param,
        1,
        NULL
    );

    request->send(response);
}

// ================================================================
// 📌 5. ФУНКЦІЯ ІНІЦІАЛІЗАЦІЇ ВСІХ ХЕНДЛЕРІВ
// ================================================================
void initWebHandlers(AsyncWebServer &server) {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/logs", HTTP_GET, handleLogs);
    server.on("/log_viewer", HTTP_GET, handleLogViewer);
    server.on("/save_now", HTTP_GET, handleSaveNow);
}


🔗 У main.cpp тепер достатньо:

cpp
Копіювати
Редагувати
#include "Handlers_Stream.h"

// ...
initWebHandlers(server);
