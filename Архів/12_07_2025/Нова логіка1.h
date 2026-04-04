static bool archiveDoneToday = false;
static bool fileCreated = false;
static String lastInitDate = "";

String currentTime = getCurrentTime();   // "HH:MM"
String today = getTodayDate();           // "YYYY-MM-DD"

// 🕓 АРХІВАЦІЯ ≥ 23:58 (один раз на добу)
if (currentTime >= "23:58" && !archiveDoneToday) {
  Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

  bool *param = new bool(true);  // після архівації буде очищення
  xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

  archiveDoneToday = true;
  Serial.println("✅ Архівування розпочато у фоновому режимі.");
}

// 🕛 Створення нового CSV >= 00:01
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


////
І залишаємо clearHistoryData() тільки в saveHistoryTask():
cpp
Копіювати
Редагувати
if (clearAfterSave) {
  clearHistoryData();        // ✅ очищення ТІЛЬКИ після успішного збереження
  clearOldLogs(3);
  Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  
  
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

    bool ok = file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    if (!ok) {
      Serial.printf("❌ Помилка запису #%d: %s\n", i, d.timeStr.c_str());
    }
    written++;
  }

  file.flush();   // 💾 Гарантований запис
  file.close();   // 🔐 Безпечне закриття
  delay(50);      // ⏳ Запас часу для SPIFFS

  Serial.printf("✅ Архів завершено: %s (%d записів)\n", filename.c_str(), written);

  if (clearAfterSave) {
    clearHistoryData();
    clearOldLogs(30);
    Serial.println("🧹 Старі архіви очищено (залишено останні 30 днів)");
  }

  vTaskDelete(NULL);
}


void loop() {
  static bool archiveDoneToday = false;
  static bool fileCreated = false;
  static String lastInitDate = "";

  String currentTime = getCurrentTime();   // "HH:MM"
  String today = getTodayDate();           // "YYYY-MM-DD"

  // 🕓 АРХІВАЦІЯ ≥ 23:58 (один раз на добу)
  if (currentTime >= "23:58" && !archiveDoneToday) {
    Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

    bool *param = new bool(true);  // автоматичне з очищенням
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveDoneToday = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 Створення нового CSV >= 00:01
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

  // ... інша логіка: сенсори, дисплей, веб
}

////////////////////////////////
void loop() {
  // --- Статичні флаги та змінна дати ---
  static bool archiveDoneToday = false;     // Чи вже виконане архівування сьогодні
  static bool fileCreated = false;          // Чи вже створено файл архіву на сьогодні
  static String lastInitDate = "";          // Дата, коли востаннє створювали файл

  // --- Поточний час та дата ---
  String currentTime = getCurrentTime();    // Очікується формат "HH:MM"
  String today = getTodayDate();            // Очікується формат "YYYY-MM-DD"

  // ================================================================
  // 🕓 АВТОМАТИЧНЕ АРХІВУВАННЯ О 23:58
  // ================================================================
  if (currentTime >= "23:58" && !archiveDoneToday) {
    // ✅ Умова: час >= 23:58 і архів ще не створено сьогодні

    Serial.println("🕓 Поточний час >= 23:58 — запускаю архівацію...");

    // Параметр true означає: очищення history[] і видалення старих архівів
    bool *param = new bool(true);

    // Запуск фонового завдання архівації
    xTaskCreatePinnedToCore(
      saveHistoryTask,     // функція, яка буде виконуватись
      "SaveHistory",       // ім’я завдання
      8192,                // розмір стека (байти)
      param,               // параметр (вказівник на bool)
      1,                   // пріоритет
      NULL,                // дескриптор (не потрібен)
      1                    // ядро CPU (ESP32 має два)
    );

    archiveDoneToday = true; // Позначити, що архівація вже запускалась
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // ================================================================
  // 🕛 СТВОРЕННЯ ФАЙЛУ ДЛЯ НОВОГО ДНЯ після 00:01
  // ================================================================
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    // ✅ Умова: новий день і ще не створено файл

    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        // Додаємо UTF-8 BOM і заголовок CSV
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
    lastInitDate = today; // Запам'ятовуємо дату створення
  }

  // ================================================================
  // ♻️ СКИДАННЯ ФЛАГІВ ПІСЛЯ 01:00
  // ================================================================
  if (currentTime >= "01:00") {
    // Скидаємо флаги, щоб наступного дня все почалось знову

    if (archiveDoneToday || fileCreated) {
      Serial.println("♻️ Час >= 01:00 — скидаю флаги архівації/створення");
    }

    archiveDoneToday = false;
    fileCreated = false;
  }

  // ================================================================
  // ... РЕШТА ЛОГІКИ (сенсори, дисплей, обробка веб-запитів)
  // ================================================================
}



 