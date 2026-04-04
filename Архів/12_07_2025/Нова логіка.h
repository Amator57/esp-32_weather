void loop() {
  static bool archiveDoneToday = false;
  static bool fileCreated = false;
  static String lastInitDate = "";

  String currentTime = getCurrentTime();   // "HH:MM"
  String today = getTodayDate();           // "YYYY-MM-DD"

  // 🕓 Архівування >= 23:58
  if (currentTime >= "23:58" && !archiveDoneToday) {
    Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

    bool *param = new bool(true);
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveDoneToday = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 Створення нового CSV-файлу >= 00:01
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
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

  // ♻️ Скидання флагів після 01:00
  if (currentTime >= "01:00") {
    if (archiveDoneToday || fileCreated) {
      Serial.println("♻️ Час >= 01:00 — скидаю флаги архівації/створення");
    }
    archiveDoneToday = false;
    fileCreated = false;
  }

  // ... решта логіки loop(): сенсори, дисплей, сервер тощо
}
 