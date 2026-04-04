#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define MAX_MEASUREMENTS 1440

struct Measurement {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

Measurement history[MAX_MEASUREMENTS];
int historyCount = 0;

Adafruit_BME280 bme;
unsigned long currentMillis;

String getCurrentTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "1970-01-01 00:00:00";
  }
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void storeToHistory(float temp, float hum, float pres, const String& timeStr) {
  // Уникнення дублювання
  for (int i = 0; i < historyCount; ++i) {
    if (history[i].timeStr == timeStr) {
      Serial.println("⚠️ Запис з таким часом вже існує, пропускаємо.");
      return;
    }
  }

  // Якщо переповнення — видаляємо найстаріший
  if (historyCount >= MAX_MEASUREMENTS) {
    for (int i = 1; i < MAX_MEASUREMENTS; ++i) {
      history[i - 1] = history[i];
    }
    historyCount = MAX_MEASUREMENTS - 1;
  }

  history[historyCount].temperature = temp;
  history[historyCount].humidity = hum;
  history[historyCount].pressure = pres;
  history[historyCount].timeStr = timeStr;
  historyCount++;

  Serial.printf("💾 Збережено запис #%d: %s | %.2f°C %.2f%% %.2f hPa\n",
                historyCount, timeStr.c_str(), temp, hum, pres);
}

void performHistoryFileSave() {
  File file = SPIFFS.open("/history.csv", FILE_APPEND);
  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл історії для дозапису!");
    return;
  }

  for (int i = 0; i < historyCount; ++i) {
    if (history[i].timeStr != "") {
      file.printf("%s,%.2f,%.2f,%.2f\n",
                  history[i].timeStr.c_str(),
                  history[i].temperature,
                  history[i].humidity,
                  history[i].pressure);
    }
  }

  file.close();
  historyCount = 0;  // Очищуємо буфер після збереження
  Serial.println("✅ Дані історії збережено у файл і буфер очищено.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Помилка ініціалізації SPIFFS");
    return;
  }

  if (!bme.begin(0x76)) {
    Serial.println("❌ BME280 не знайдено!");
    while (1);
  }

  Serial.println("✅ Ініціалізація завершена.");
}

void loop() {
  currentMillis = millis();

  static uint32_t lastUpdate = 0;
  if (currentMillis - lastUpdate > 60000) {  // раз на 60 с
    lastUpdate = currentMillis;

    float lastTemp = bme.readTemperature();
    float lastHum = bme.readHumidity();
    float lastPress = bme.readPressure();

    String timeStr = getCurrentTime();
    storeToHistory(lastTemp, lastHum, lastPress, timeStr);

    Serial.println("📈 Дані збережено з BME280");

    // Діагностика памʼяті
    Serial.printf("🧠 Вільно Heap: %u байт\n", ESP.getFreeHeap());
  }

  delay(1000);
}

// !8.07.2025 Мій алгоритм

// ... решта #include залишаються без змін
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>

// === 🔧 ЗМІННІ ДЛЯ ІНДЕКСУВАННЯ АРХІВАЦІЇ ===
int historyIndex = 0;
int lastArchivedIndex = 0;

// Структура для зберігання даних з BME280
struct BMEData {
  String timeStr;
  float temperature;
  float humidity;
  float pressure;
};

// Масив для збереження історії
const int maxHistorySize = 1440;
BMEData history[maxHistorySize];

// === 📝 ФУНКЦІЯ АРХІВАЦІЇ ===
void performHistoryFileSave() {
  String filename = "/log/" + getCurrentDate() + ".csv";
  File file;

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    SPIFFS.mkdir("/log");
  }

  bool fileExists = SPIFFS.exists(filename);
  file = SPIFFS.open(filename, FILE_APPEND);

  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису архіву!");
    return;
  }

  if (!fileExists) {
    file.println("Time,Temperature,Humidity,Pressure");
  }

  Serial.println("🧪 Запис у файл архіву...");
  int recordsWritten = 0;

  for (int i = lastArchivedIndex; i < historyIndex; i++) {
    BMEData entry = history[i];
    String line = entry.timeStr + "," + String(entry.temperature, 2) + "," + String(entry.humidity, 2) + "," + String(entry.pressure, 2);
    file.println(line);
    recordsWritten++;
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), recordsWritten);

  lastArchivedIndex = historyIndex;
}

// === 🔁 СКИДАННЯ ІНДЕКСІВ НА ПОЧАТКУ ДОБИ ===
void resetHistoryForNewDay() {
  historyIndex = 0;
  lastArchivedIndex = 0;
}

// === 🌡️ ФУНКЦІЯ ДОДАВАННЯ ЗАПИСУ ДО HISTORY[] ===
void saveCurrentReadingToHistory(float temp, float hum, float pres, const String& timeStr) {
  if (historyIndex >= maxHistorySize) return;
  history[historyIndex++] = { timeStr, temp, hum, pres };
}

// ... решта функцій, loop(), setup() — залишаються без змін
