void loop() {
  static String lastSavedDate = "";
  static String lastInitDate = "";

  String currentTime = getCurrentTime();  // формат "HH:MM"
  String today = getTodayDate();

  // 🕓 О 23:59 — запуск фонового архівування
  if (currentTime == "23:59" && today != lastSavedDate) {
    Serial.println("🕓 23:59 — Архівую поточні дані у " + today);
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, NULL, 1, NULL, 1);
    lastSavedDate = today;
  }

  // 🕛 О 00:01 — створення нового файлу, якщо ще не існує
  if (currentTime == "00:01" && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";
    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);
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

  // ... решта вашої логіки loop() ...
}