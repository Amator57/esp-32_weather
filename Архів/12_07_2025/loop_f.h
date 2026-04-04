void loop() {
  unsigned long currentMillis = millis();

    static String lastArchiveDate = "";  // дата останнього архіву
  static String lastInitDate = "";       // дата останнього створеного файлу
  static bool archiveStarted = false;    // 🔁 Щоб не запускати архів кілька разів
  static bool fileCreated = false;       // 🔁 Щоб файл створився лише 1 раз
  String currentTime = getCurrentTime(); // "HH:MM"
  String today = getTodayDate();         // "YYYY-MM-DD"

  // 🕓 Автоматичне архівування >= 23:58
  if (currentTime >= "23:58" && !archiveStarted && lastArchiveDate != today) {
    Serial.println("🕓 Поточний час >= 23:58 — починаю архівацію...");

    bool *param = new bool(true);  // true = очищення + видалення старих
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, param, 1, NULL, 1);

    archiveStarted = true;
    lastArchiveDate = today;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // 🕛 СТВОРЕННЯ АРХІВНОГО ФАЙЛУ НА НОВУ ДОБУ (00:01+)
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

 /* //не перевірений варіант. Здається має проблему із скиданням флага архівування 11.07.2025
  static bool archiveStarted = false;  // 🔁 Щоб не запускати архів кілька разів
  static bool fileCreated = false;     // 🔁 Щоб файл створився лише 1 раз
  static String lastInitDate = "";     // 🕛 Для уникнення повторного створення

  String currentTime = getCurrentTime();   // Формат "HH:MM"
  String today = getTodayDate();           // Формат "YYYY-MM-DD"

  // ================================================================
  // 🕓 НАДІЙНЕ АВТОМАТИЧНЕ АРХІВУВАННЯ О 23:58
  // ================================================================
  if (currentTime >= "23:58" && !archiveStarted) {
    Serial.println("🕓 Поточний час >= 23:58 — починаю архівацію поточних даних...");

    bool *param = new bool(true);  // true → після збереження очищуємо масив
    xTaskCreatePinnedToCore(
      saveHistoryTask,
      "SaveHistory",
      8192,
      param,
      1,
      NULL,
      1
    );

    archiveStarted = true;
    Serial.println("✅ Архівування розпочато у фоновому режимі.");
  }

  // ================================================================
  // 🕛 СТВОРЕННЯ АРХІВНОГО ФАЙЛУ НА НОВУ ДОБУ (00:01+)
  // ================================================================
  if (currentTime >= "00:01" && !fileCreated && today != lastInitDate) {
    String filename = "/log/" + today + ".csv";

    if (!SPIFFS.exists(filename)) {
      File f = SPIFFS.open(filename, FILE_WRITE);
      if (f) {
        f.write(0xEF); f.write(0xBB); f.write(0xBF);  // UTF-8 BOM
        f.println("Time,Temperature,Humidity,Pressure");
        f.close();
        Serial.println("📁 Створено новий файл архіву: " + filename);
      } else {
        Serial.println("❌ Помилка створення нового архівного файлу!");
      }
    } else {
      Serial.println("ℹ️ Файл архіву на сьогодні вже існує: " + filename);
    }

    fileCreated = true;
    lastInitDate = today;
  }

  //================++++++++++++++++++++++++

 
  // ================================================================
  // 🔄 СКИДАННЯ ФЛАГІВ ПІСЛЯ 01:00
  // ================================================================ 
  if (currentTime >= "01:00") {
    if (archiveStarted || fileCreated) {
      Serial.println("♻️ Час > 01:00 — скидаю флаги архівації/створення файлу.");
    }
    archiveStarted = false;
    fileCreated = false;
  }
 */
  // 🔽 Далі розміщується інша логіка: сенсори, дисплей, веб-сервер тощо...


/*
   // 🔧 ДОДАТКОВИЙ блок контролю часу, вставити НА САМОМУ ПОЧАТКУ
  static String lastSavedDate = "";
  static String lastInitDate = "";

  String currentTime = getCurrentTime();  // формат "HH:MM"
  String today = getTodayDate();

  if (currentTime == "23:59" && today != lastSavedDate) {
    Serial.println("🕓 23:59 — Архівую поточні дані у " + today);
    xTaskCreatePinnedToCore(saveHistoryTask, "SaveHistory", 8192, NULL, 1, NULL, 1);
    lastSavedDate = today;
  }

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
*/
  // 🔽 Далі залишається ВАША основна логіка loop():
  // - опитування сенсорів
  // - оновлення дисплея
  // - обробка веб-запитів
  // - оновлення графіків і т.д


  // ==========================================================
  // Блок ОНОВЛЕННЯ ДАНИХ СЕНСОРІВ ТА ЗБЕРЕЖЕННЯ В ІСТОРІЮ
  // Цей блок перевіряє, чи минула 5 хвилин (300000 мс) з останнього оновлення даних.
  // Він працює незалежно від того, що відображається на екрані.
  // ==========================================================
  static uint32_t lastUpdate = 0;
  if (currentMillis - lastUpdate > 60000) {
    lastUpdate = currentMillis;
    lastTemp = bme.readTemperature();
    lastHum = bme.readHumidity();
    lastPress = bme.readPressure();

    String timeStr = getCurrentTime();
    storeToHistory(lastTemp, lastHum, lastPress, timeStr);

    Serial.println("Дані ВМЕ280 оновлено і збережено");
  }
// У вашій функції loop() знайдіть цей блок:
// ==========================================================
// Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
// ==========================================================
/*
  //static String lastSavedDate;
  //String today = getTodayDate();
  if (today != lastSavedDate) {
    Serial.println("Зміна дати виявлена! Запускаю архівування та очищення."); // Для налагодження

    // ЗАМІНІТЬ ЦЕЙ РЯДОК:
    // performHistoryFileSave();

    // НА ЦЕЙ БЛОК:
    xTaskCreate(
      saveHistoryTask,       // Функція, що буде виконуватись як завдання
      "SaveHistory",         // Назва завдання (для дебагу)
      8192,                  // Розмір стека в байтах
      NULL,                  // Параметр, що передається в завдання (тут не потрібен)
      1,                     // Пріоритет завдання (0 - найнижчий. 1 - вже робочий)
      NULL                   // Дескриптор завдання (якщо потрібно його контролювати)
    );
//Test
    clearOldLogs(3);        // Очистити старі файли логів (3 дні)
    clearHistoryData();      // Очистити масив історії для нового дня
    lastSavedDate = today;   // Оновити останню збережену дату
  }
*/
/*
  // ==========================================================
  // Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
  // ==========================================================
  static String lastSavedDate;
  String today = getTodayDate();
  if (today != lastSavedDate) {
    performHistoryFileSave();
    clearOldLogs(30);
    clearHistoryData();
    lastSavedDate = today;
  }
*/
  // ==========================================================
  // БЛОК КЕРУВАННЯ ДИСПЛЕЄМ (НОВА МАШИНА СТАНІВ ДЛЯ РОТАЦІЇ)
  // Всі режими виводяться ОДНОРАЗОВО при вході в режим.
  // ==========================================================

  // --- Фаза 1: Оновлення списку активних режимів для ротації ---
  // Оновлювати список активних режимів кожні 500 мс (залишаємо як є)
  static unsigned long lastModeListUpdate = 0;
  const unsigned long MODE_LIST_UPDATE_INTERVAL = 500; 

  if (currentMillis - lastModeListUpdate >= MODE_LIST_UPDATE_INTERVAL) {
    activeDisplayModes.clear(); 

    if (switch4) activeDisplayModes.push_back(MODE_TOLYA_VITA);
    if (switch6) activeDisplayModes.push_back(MODE_UKRAINIAN);
    if (switch7) activeDisplayModes.push_back(MODE_VIVAT);
    if (switch5) activeDisplayModes.push_back(MODE_SLAVA_UKRAINI);
    if (switch1) activeDisplayModes.push_back(MODE_DATETIME);
    if (switch3) activeDisplayModes.push_back(MODE_SHEVCHENKO);
    if (switch2) activeDisplayModes.push_back(MODE_BME_DATA);

    rotationActive = !activeDisplayModes.empty(); 
    lastModeListUpdate = currentMillis; 
  }

  // === ВАЖЛИВО: Зберегти поточний режим перед тим, як він може змінитися ===
  previousDisplayMode = currentDisplayMode; 

  // --- Фаза 2: Керування відображенням на основі списку ---
  switch (currentDisplayMode) {
    case MODE_NONE:
      if (rotationActive) {
        currentRotationIndex = 0; 
        currentDisplayMode = activeDisplayModes[currentRotationIndex];
        displayModeStartTime = currentMillis;
        
        // **Викликаємо executeDisplayMode ОДИН РАЗ при вході в новий режим**
        executeDisplayMode(currentDisplayMode); 
      } else {
        displayDefaultBackground();
      }
      break;

    // Спільна логіка для всіх сюжетів, що ротуються
    case MODE_TOLYA_VITA:
    case MODE_UKRAINIAN:
    case MODE_VIVAT:
    case MODE_SLAVA_UKRAINI:
    case MODE_DATETIME:
    case MODE_SHEVCHENKO:
    case MODE_BME_DATA:
      // В цьому блоці більше НЕМАЄ періодичних викликів displayDateTime(), displayBMEData(), displayTolyaVita() тощо.
      // Вміст режиму виводиться лише один раз при зміні currentDisplayMode.

      if (currentMillis - displayModeStartTime >= getDisplayDuration(currentDisplayMode)) {
        // Час поточного сюжету вичерпався. Переходимо до наступного або до MODE_NONE.
        
        if (rotationActive) { 
          // Визначити наступний режим у ротації
          currentRotationIndex = (currentRotationIndex + 1) % activeDisplayModes.size();
          DisplayMode nextMode = activeDisplayModes[currentRotationIndex];

          // **Логіка керування шрифтом My_ariali_26 (якщо потрібен):**
          // Якщо режим DATETIME використовує спеціальний шрифт, а інші ні.
          // Цей блок відповідає за те, щоб шрифт був завантажений ТІЛЬКИ для DATETIME
          // і вивантажений, коли виходимо з DATETIME.
          // Якщо ви хочете повністю ВИКЛЮЧИТИ встановлення цього шрифту,
          // видаліть цей if/else if блок повністю, і також приберіть loadFont/unloadFont
          // з функції executeDisplayMode, якщо вони там є.
          if (nextMode == MODE_DATETIME) {
              if (previousDisplayMode != MODE_DATETIME) {
                  tft.loadFont(My_ariali_26); // Завантажуємо шрифт тільки якщо переходимо в DATETIME
              }
          } else {
              if (previousDisplayMode == MODE_DATETIME) {
                  tft.unloadFont(); // Вивантажуємо шрифт тільки якщо виходимо з DATETIME
              }
          }
          //++++++++++++++++++++++++++++++++++++++

          currentDisplayMode = nextMode;
          displayModeStartTime = currentMillis;
          // **Викликаємо executeDisplayMode ОДИН РАЗ для нового режиму**
          executeDisplayMode(currentDisplayMode); 

        } else {
          // Якщо після завершення сюжету не залишилося активних перемикачів, повертаємося до MODE_NONE
          // **Вивантажуємо шрифт, якщо виходимо з DATETIME в MODE_NONE**
          if (previousDisplayMode == MODE_DATETIME) {
              tft.unloadFont();
          }
          currentDisplayMode = MODE_NONE;
        }
      }
      break;
  }
}


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

  File file = SPIFFS.open(filename, FILE_APPEND);  // ✅ ДОПИСУЄМО
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису");
    vTaskDelete(NULL);
    return;
  }

  // Якщо файл новий — додати заголовок
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
  Serial.printf("✅ Архівацію  завершено: %s (%d записів)\n", filename.c_str(), written);

  
  if (clearAfterSave) {
    clearHistoryData();        // очищення масиву поточних даних
    clearOldLogs(3);          // 🔥 очищення архівів старших за 3 днів
    Serial.println("🧹 Старі архіви очищено (залишено останні 3 дні)");
  }

  vTaskDelete(NULL);
}