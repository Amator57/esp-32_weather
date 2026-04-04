void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";

  Serial.println("📦 Запуск архівації...");
  Serial.println("➡️  Файл: " + filename);

  if (!SPIFFS.exists("/log")) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  }

  // 🧪 Діагностика масиву history[]
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr != "") {
      Serial.printf("  #%d %s | %.1f°C %.1f%% %.1f hPa", i, d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
      nonEmpty++;
    }
  }
  Serial.printf("ℹ️ Усього активних записів: %d
", nonEmpty);
  if (nonEmpty == 0) {
    Serial.println("⚠️ Масив history[] порожній. Архівація не буде виконана.");
    vTaskDelete(NULL);
    return;
  }

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f
", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s
", i, d.timeStr.c_str());
    }
    written++;
  }

  file.flush();  // 💾 Гарантоване скидання буфера
  file.close();  // 🔐 Закриття файлу
  delay(50);     // 💤 Дати SPIFFS завершити запис

  Serial.printf("✅ Архів завершено: %s (%d записів)
", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData(); // ✅ автоматично очищаємо
    clearOldLogs(30);	// ✅ і видаляємо старі архіви
    Serial.println("🧹 Старі архіви очищено (залишено останні 30 днів)");
  }

  vTaskDelete(NULL);
}