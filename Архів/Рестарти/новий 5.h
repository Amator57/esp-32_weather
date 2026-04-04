Програма ChatGPT сказала:
Ось повністю зведений та чистий web_handlers.h, який можна одразу класти в проєкт і підключати в main.cpp.
Він замінює всі старі Handlers.h, Handlers_1.h і дублікати функцій.

cpp
Копіювати
Редагувати
#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

//
// ✅ Хендлер головної сторінки (index.html)
// Повертає базову HTML-сторінку з меню або інфо.
// 
void handleRootPage(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");

  response->print(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32</title></head><body>"));
  response->print(F("<h2>Головна сторінка ESP32</h2>"));
  response->print(F("<ul>"));
  response->print(F("<li><a href='/logs'>📂 Переглянути архіви</a></li>"));
  response->print(F("<li><a href='/log_viewer'>📊 Відкрити переглядач графіків</a></li>"));
  response->print(F("</ul>"));
  response->print(F("</body></html>"));

  request->send(response);
}

//
// ✅ Хендлер сторінки зі списком лог-файлів
// Виводить перелік усіх CSV-файлів у директорії /log з посиланнями
//
void handleLogList(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");

  response->print(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Логи ESP32</title></head><body>"));
  response->print(F("<h2>Доступні архіви:</h2><ul>"));

  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    response->print(F("<li>Немає доступних лог-файлів.</li>"));
  } else {
    File file = root.openNextFile();
    while (file) {
      String fullPath = file.name();    // /log/2025-07-22.csv
      if (!fullPath.endsWith(".csv")) {
        file = root.openNextFile();
        continue;
      }

      String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1); // 2025-07-22.csv
      String datePart = fileName;
      datePart.replace(".csv", ""); // 2025-07-22

      // ✅ Формуємо HTML-рядок: [Графік] і [CSV]
      response->print("<li>");
      response->print("<a href='/log_viewer?date=" + datePart + "'>📊 " + datePart + "</a> ");
      response->print("(<a href='/log/" + fileName + "'>csv</a>)");
      response->print("</li>");

      file = root.openNextFile();
    }
  }

  response->print(F("</ul><p><a href='/'>На головну</a></p></body></html>"));
  request->send(response);
}

//
// ✅ Хендлер сторінки log_viewer (вибір дати + JS для побудови графіків)
//
void handleLogViewer(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");

  response->print(F("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Переглядач логів</title>"));
  response->print(F("<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>"));

  response->print(F("<h2>Перегляд графіків</h2>"));
  response->print(F("<input type='date' id='datePicker'>"));
  response->print(F("<button onclick='loadData()'>Завантажити</button>"));
  response->print(F("<div><canvas id='tempChart' width='400' height='200'></canvas></div>"));
  response->print(F("<div><canvas id='humChart' width='400' height='200'></canvas></div>"));
  response->print(F("<div><canvas id='pressChart' width='400' height='200'></canvas></div>"));

  // ✅ Скрипт JS для завантаження даних з CSV і побудови графіків
  response->print(F("<script>"
                    "function loadData(){"
                    "let date=document.getElementById('datePicker').value;"
                    "if(!date){alert('Оберіть дату');return;}"
                    "fetch('/log/'+date+'.csv')"
                    ".then(r=>r.text())"
                    ".then(text=>{"
                    "let lines=text.split('\\n');"
                    "let times=[],temps=[],hums=[],press=[];"
                    "for(let i=1;i<lines.length;i++){"
                    "let parts=lines[i].split(',');"
                    "if(parts.length<4) continue;"
                    "times.push(parts[0]);"
                    "temps.push(parseFloat(parts[1]));"
                    "hums.push(parseFloat(parts[2]));"
                    "press.push(parseFloat(parts[3]));}"
                    "drawCharts(times,temps,hums,press);});}"
                    
                    "function drawCharts(t,temps,hums,press){"
                    "new Chart(document.getElementById('tempChart').getContext('2d'),"
                    "{type:'line',data:{labels:t,datasets:[{label:'Температура',data:temps,borderColor:'red',fill:false}]}});"
                    "new Chart(document.getElementById('humChart').getContext('2d'),"
                    "{type:'line',data:{labels:t,datasets:[{label:'Вологість',data:hums,borderColor:'blue',fill:false}]}});"
                    "new Chart(document.getElementById('pressChart').getContext('2d'),"
                    "{type:'line',data:{labels:t,datasets:[{label:'Тиск',data:press,borderColor:'green',fill:false}]}});}"
                    "</script>"));

  response->print(F("</body></html>"));
  request->send(response);
}

//
// ✅ ЄДИНА ФУНКЦІЯ РЕЄСТРАЦІЇ ВСІХ ХЕНДЛЕРІВ
//
void registerWebHandlers(AsyncWebServer &server) {

  // 🏠 Головна сторінка
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRootPage(request);
  });

  // 📋 Сторінка зі списком логів
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleLogList(request);
  });

  // 📈 Переглядач логів
  server.on("/log_viewer", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleLogViewer(request);
  });

  // 📂 Видача статичних файлів із SPIFFS (JS, CSS, зображення)
  server.serveStatic("/", SPIFFS, "/")
        .setDefaultFile("index.html")
        .setCacheControl("max-age=600");

  // ❌ 404 якщо нічого не знайдено
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "404: Сторінка не знайдена");
  });
}

#endif // WEB_HANDLERS_H
🔧 Що робить цей файл
✅ Містить усі три основні хендлери:

handleRootPage() – головна сторінка (index).

handleLogList() – сторінка зі списком CSV логів.

handleLogViewer() – сторінка з JavaScript для побудови графіків.

✅ Має одну-єдину функцію registerWebHandlers(), яка реєструє все.

✅ Видалено всі дублікати, старі версії і зайві визначення.

📌 Як підключити
У main.cpp:

cpp
Копіювати
Редагувати
#include "web_handlers.h"

// ...

void setup() {
  // ... ініціалізація WiFi, SPIFFS і т.д.

  registerWebHandlers(server);  // ✅ підключення всіх хендлерів
  server.begin();
}
Хочеш, щоб я одразу додав цей файл у готовий ZIP-проєкт PlatformIO з мінімальним прикладом, щоб ти міг протестувати?







