void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  // Створити директорію /log, якщо потрібно
  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  // Підрахунок не порожніх записів
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa\n", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }

  Serial.printf("ℹ️ Усього активних записів: %d\n", nonEmpty);
  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  // Відкриття файлу на дописування
  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл порожній — записати заголовок
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  // Запис усіх записів у CSV
  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData();
    clearOldLogs(30);
    Serial.println("🧹 Старі архіви очищено (залишено останні 30 днів)");
  }

  vTaskDelete(NULL);
}

////////////////////////////////////////////////////
void loop() {
  static bool archiveDoneToday = false;     // Чи вже архівували сьогодні
  static bool fileCreated = false;          // Чи вже створили файл
  static String lastInitDate = "";          // Дата останнього створення CSV
  static String lastDateReset = "";         // Дата останнього скидання флагів

  String currentTime = getCurrentTime();    // Формат: "HH:MM"
  String today = getTodayDate();            // Формат: "YYYY-MM-DD"

  // ♻️ Скидання флагів — лише 1 раз на добу, після 00:01 і до 23:58
  if (currentTime >= "00:01" && currentTime < "23:58" && today != lastDateReset) {
    Serial.println("♻️ Скидання флагів архівації та створення — нова дата!");
    archiveDoneToday = false;
    fileCreated = false;
    lastDateReset = today;
  }

  // 🕓 АРХІВАЦІЯ після 23:58
  if (currentTime >= "23:58" && !archiveDoneToday) {
    Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

    static bool clearParam = true;  // Статичний параметр
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, &clearParam, 1, NULL, 1);

    archiveDoneToday = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 Створення нового CSV-файлу після 00:01
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF); // UTF-8 BOM
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Створено новий архівний файл: " + filename);
      } else {
        Serial.println("❌ Помилка створення архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл архіву вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  // ... інша логіка: сенсори, дисплей, веб-сервер
}
////////////////////////////
String getCurrentTime() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  // ✅ Завжди два символи
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  return String(buffer);  // наприклад: "09:05", "13:27"
}
