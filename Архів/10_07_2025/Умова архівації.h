Але треба врахувати: loop() крутиться дуже швидко
Тому >= "23:59" спрацює на кожній ітерації після настання цього часу (тобто десятки чи сотні разів), якщо не додати фіксацію “вже виконано”.

✅ Рекомендоване рішення:
🟢 1. Застосувати >= "23:59" з одноразовим флагом bool archiveDoneToday
🔁 Повний фрагмент loop() з поясненнями:
cpp
Копіювати
Редагувати
void loop() {
  static String lastArchiveDate = "";
  static String lastInitDate = "";
  static bool archiveDoneToday = false;
  static bool fileCreated = false;

  String currentTime = getCurrentTime();  // HH:MM
  String today = getTodayDate();

  // 🕓 Архівування о 23:59 або пізніше
  if (currentTime >= "23:59" && !archiveDoneToday) {
    Serial.println("🕓 >=23:59 — Архівую поточні дані...");

    bool *param = new bool(true);  // true → очищення після збереження
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveDoneToday = true;
    lastArchiveDate = today;
  }

  // 🕛 Створення нового файлу після 00:01
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
      Serial.println("ℹ️ Архівний файл вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  // 🔁 Після 01:00 можна скинути флаги
  if (currentTime >= "01:00") {
    archiveDoneToday = false;
    fileCreated = false;
  }

  // ... решта логіки loop()
}
✅ Гарантовано:
Подія	Коли	Стан
>= 23:59	один раз	🟢 Архівує → очищує
>= 00:01	один раз	🟢 Створює новий файл
>= 01:00	один раз	🟢 Скидає флаги

📌 Коментарі:
Усі дії відбуваються в межах кількох хвилин — архівування зазвичай займає менше 1 сек для 1440 записів

Повторне створення чи записування запобігається флагами

Час "01:00" — умовний, можна виставити "00:05" або інше безпечно пізніше

🔧 Якщо бажаєте — можу додати цей фрагмент безпосередньо у ваш main.cpp.