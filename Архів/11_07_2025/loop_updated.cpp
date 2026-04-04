void loop() {
  static String lastArchiveDate = "";  // дата останнього архіву
  static String lastInitDate = "";     // дата останнього створеного файлу
  static bool archiveStarted = false;
  static bool fileCreated = false;

  String currentTime = getCurrentTime();   // "HH:MM"
  String today = getTodayDate();           // "YYYY-MM-DD"

  // 🕓 Автоматичне архівування >= 23:58
  if (currentTime >= "23:58" && !archiveStarted && lastArchiveDate != today) {
    Serial.println("🕓 Поточний час >= 23:58 — починаю архівацію...");

    bool *param = new bool(true);  // true = очищення + видалення старих
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveStarted = true;
    lastArchiveDate = today;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 Створення нового файлу >= 00:01
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Новий архівний файл створено: " + filename);
      } else {
        Serial.println("❌ Помилка створення архівного файлу");
      }
    } else {
      Serial.println("ℹ️ Файл архіву вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  // 🧹 Скидання флагів лише якщо дата ЗМІНИЛАСЯ
  if (today != lastArchiveDate) {
    if (archiveStarted || fileCreated) {
      Serial.println("♻️ Нова дата — скидаю флаги архівації та створення.");
    }
    archiveStarted = false;
    fileCreated = false;
  }

  // ... решта логіки: сенсори, дисплей, сервер тощо
}