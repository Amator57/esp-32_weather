// === main.cpp ===
// ✅ Оновлений код з інтеграцією safeAbort перед критичними помилками
// Збережено повний функціонал, додано детальне логування перед аварійним завершенням

#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <set>

// 🔐 Макрос та функція safeAbort для логування перед crash
#define SAFE_ABORT(msg) safeAbort(__FILE__, __LINE__, msg)

void safeAbort(const char* file, int line, const char* msg = "") {
  Serial.printf("\n🚨 АВАРІЙНЕ ЗАВЕРШЕННЯ\nФайл: %s\nРядок: %d\nПричина: %s\n", file, line, msg);
  delay(200);  // ⏳ Забезпечити виведення в монітор
  abort();     // ⛔ Аварійне завершення
}

// === Далі йде наявний функціонал ===

Adafruit_BME280 bme;
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("🔌 Запуск ESP32...");

  // 📁 Ініціалізація SPIFFS
  if (!SPIFFS.begin(true)) {
    SAFE_ABORT("Не вдалося ініціалізувати SPIFFS");
  }

  // 📡 Підключення до Wi-Fi (якщо потрібно)
  WiFi.begin("yourSSID", "yourPASSWORD");
  Serial.print("🌐 Підключення до Wi-Fi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tries++;
    if (tries > 20) {
      SAFE_ABORT("Wi-Fi не підключено — перевищено кількість спроб");
    }
  }
  Serial.println("✅ Wi-Fi підключено");

  // 🌡 Ініціалізація BME280
  if (!bme.begin(0x76)) {
    SAFE_ABORT("Не вдалося знайти BME280 за адресою 0x76");
  }

  // 🌐 Запуск веб-сервера
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.begin();
  Serial.println("🌍 Сервер запущено на порту 80");
}

void loop() {
  // Тут може бути логіка зчитування з BME280 або інша
}
