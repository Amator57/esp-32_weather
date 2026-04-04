// ==========================================================
// Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
// ==========================================================
  static String lastSavedDate;
  String today = getTodayDate();
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
  
  
  // ==== Нова функція: Основний функціонал збереження історії у файл ====
// Ця функція містить код, який безпосередньо записує дані в SPIFFS.
// Вона не приймає аргументів і не має нескінченних циклів чи затримок,
// тому її можна безпечно викликати з void loop().
void performHistoryFileSave() {
  File file = SPIFFS.open("/history.csv", FILE_APPEND); // Відкриваємо файл для дозапису
  if (!file) {
    Serial.println("❌ Помилка: Не вдалося відкрити файл історії для дозапису!");
    return; // Якщо не вдалося відкрити, виходимо
  }

  // Припускаємо, що 'history' та 'MAX_MEASUREMENTS' є глобально доступними
  // Цей цикл записує всі дані з масиву 'history' у файл
  for (int i = 0; i < MAX_MEASUREMENTS; ++i) {
    if (history[i].timeStr != "") { // Записуємо лише якщо є дійсні дані
      file.printf("%s,%.2f,%.2f,%.2f\n", history[i].timeStr.c_str(),
                  history[i].temperature, history[i].humidity, history[i].pressure);
    }
  }
  file.close(); // Закриваємо файл
  Serial.println("✅ Дані історії збережено у файл.");
}

void loop() {
  unsigned long currentMillis = millis();
 
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

    Serial.println("Sensor data updated and stored.");
  }
// У вашій функції loop() знайдіть цей блок:
// ==========================================================
// Блок ЩОДЕННОГО АРХІВУВАННЯ ТА ОЧИЩЕННЯ ЛОГІВ
// ==========================================================
  static String lastSavedDate;
  String today = getTodayDate();
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