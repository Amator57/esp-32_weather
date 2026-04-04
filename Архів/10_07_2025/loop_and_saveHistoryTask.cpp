// === Частина для loop() ===
void loop() {
  static String lastSavedDate = "";
  static String lastInitDate = "";

  String currentTime = getCurrentTime();  // формат: "23:59"
  String today = getTodayDate();

  // 🕓 О 23:59 — архівування у файл сьогоднішнього дня
  if (currentTime == "23:59" && today != lastSavedDate) {
    Serial.println("🕓 23:59 — Архівую поточні дані у " + today);
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, NULL, 1, NULL, 1);
    lastSavedDate = today;
  }

  // 🕛 О 00:01 — створення нового файлу для поточного дня
  if (currentTime == "00:01" && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";
    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);  // UTF-8 BOM
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Новий архівний файл створено на день: " + today);
      } else {
        Serial.println("❌ Помилка створення нового архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл на " + today + " вже існує.");
    }
    lastInitDate = today;
  }

  // решта loop()...
}


// === Фонове збереження архіву з очищенням ===
void saveHistoryTask(void *parameter) {
  String filename = "/log/" + getTodayDate() + ".csv";

  // Діагностика
  Serial.println("📦 Запуск фонового архівування...");
  Serial.println("➡️  Файл: " + filename);

  // Створити директорію /log, якщо відсутня
  if (!SPIFFS.exists("/log")) {
    SPIFFS.mkdir("/log");
  }

  File file = SPIFFS.open(filename, FILE_WRITE);  // перезапис
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  file.write(0xEF); file.write(0xBB); file.write(0xBF);
  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // Очищення масиву після завершення
  clearHistoryData();

  vTaskDelete(NULL);
}