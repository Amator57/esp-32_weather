//++++++++++++++++++++++++++++++++++++++
//Варіант від 19.07.2025 (див. нижче) Зміни в частині перевірки наявності Директорії /log 
void saveHistoryTask(void *parameter) {
  bool clearAfterSave = true;
  if (parameter != NULL) {
    clearAfterSave = *((bool*)parameter);
    delete (bool*)parameter;
  }

  String filename = "/log/" + getTodayDate() + ".csv";												 

  Serial.println("📦 Запуск архівації...");
  Serial.printf("➡️  Файл: %s\n", filename.c_str());

// ✅ Перевірка наявності директорії /log
  File testDir = SPIFFS.open("/log");
  if (!testDir || !testDir.isDirectory()) {
    Serial.println("ℹ️ Директорія /log відсутня — створюю...");
    if (!SPIFFS.mkdir("/log")) {
      Serial.println("❌ Не вдалося створити /log");
      vTaskDelete(NULL);
      return;
    }
  } else {
    Serial.println("📁 Директорія /log існує.");
  }
  testDir.close();
//Перевірка завершена

  if (clearAfterSave) {
    Serial.println("🔁 Тип: Автоматична архівація (з очищенням history[])");
  } else {
    Serial.println("🛠️ Тип: Ручна архівація (без очищення history[])");
  }
  int nonEmpty = 0;
  Serial.println("🧪 Поточні записи у history[]:");
  for (int i = 0; i < historyIndex; i++) {
    const BMEData &d = history[i];
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

  File file = SPIFFS.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  if (file.size() == 0) {
    file.write(0xEF); file.write(0xBB); file.write(0xBF);  // UTF-8 BOM
    file.println("Time,Temperature,Humidity,Pressure");
  }

  int written = 0;
  for (int i = lastSavedIndex; i < historyIndex; i++) {
    const BMEData &d = history[i];
    if (d.timeStr == "") continue;

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n",
                          d.timeStr.c_str(),
                          d.temperature,
                          d.humidity,
                          d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    } else {
      written++;
    }
  }

  file.flush();
  file.close();
  delay(50);

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  // оновлюємо лічильник
  lastSavedIndex = historyIndex;
  Serial.printf("📌 Оновлено lastSavedIndex → %d\n", lastSavedIndex);

  if (clearAfterSave) {
  clearHistoryData(); // очищення масиву поточних даних
  Serial.println("🧹 Масив history[] очищено після архівації");

  Serial.println("♻️ Виклик clearOldLogs(3)...");  // Додано
  clearOldLogs(3);  // очищення старих архівів
  Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
}
  vTaskDelete(NULL);
}