void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;

  // Якщо переданий параметр (наприклад, false для ручного запуску)
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

  File file = SPIFFS.open(filename, FILE_APPEND);  // ✅ ДОПИСУЄМО
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл новий і пустий — додаємо BOM + заголовок
  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);
    file.println("Time,Temperature,Humidity,Pressure");
  }

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

  if (clearAfterSave) {
    clearHistoryData();        // ✅ автоматично очищаємо
    clearOldLogs(30);          // ✅ і видаляємо старі архіви
  }

  vTaskDelete(NULL);
}