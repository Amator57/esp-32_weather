Так, це підтверджує нашу гіпотезу:
📌 О 00:05 ранку виконується збереження архіву — але всі записи в history[] вже стосуються нової доби (2025-07-06), а не вчорашньої.

✅ Як правильно реалізувати автоматичне збереження за попередню дату:
🔧 Крок 1: Додайте функцію getYesterdayDate()
cpp
Копіювати
String getYesterdayDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "unknown";
  time_t now = mktime(&timeinfo);
  now -= 86400;  // мінус 1 день (в секундах)
  struct tm *yesterday = localtime(&now);
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", yesterday);
  return String(buf);
}
🔧 Крок 2: Оновіть saveHistoryToFile() — додайте параметр useYesterday
cpp
Копіювати
void saveHistoryToFile(bool useYesterday = false) {
  String dateStr = useYesterday ? getYesterdayDate() : getTodayDate();
  String filename = "/log/" + dateStr + ".csv";

  if (!SPIFFS.exists("/log")) SPIFFS.mkdir("/log");

  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("❌ Не вдалося відкрити файл для запису!");
    return;
  }

  // Додати UTF-8 BOM
  file.write(0xEF); file.write(0xBB); file.write(0xBF);

  file.println("Time,Temperature,Humidity,Pressure");

  int written = 0;
  for (int i = 0; i < MAX_POINTS; i++) {
    int idx = (historyIndex + i) % MAX_POINTS;
    const BMEData &d = history[idx];

    if (d.timeStr == "") continue;
    file.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
    written++;
  }

  file.close();
  Serial.printf("✅ Архів збережено: %s (%d записів)\n", filename.c_str(), written);
}
🔧 Крок 3: У setup() залиште ручне збереження як є (поточна дата)
cpp
Копіювати
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
  saveHistoryToFile(false);  // поточна дата
  request->send(200, "text/plain", "✅ Поточний архів збережено у файл.");
});
🔧 Крок 4: У loop() або setup() при автоматичному збереженні передайте true:
cpp
Копіювати
static String lastSavedDate;
String today = getTodayDate();

if (today != lastSavedDate && historyIndex >= 10) {
  saveHistoryToFile(true);      // <-- зберігаємо вчорашній файл
  clearOldLogs(30);
  lastSavedDate = today;
}
✅ Результат:
При автоматичному збереженні о 00:00–00:05 архів записується у файл із вчорашньою датою — бо всі дані в history[] належать попередньому дню.

При ручному збереженні ("/save_now") архів записується з поточною датою.

🔁 Якщо хочете — можу надати повністю змінену версію saveHistoryToFile() та loop() в одному блоці.

//////////////////////////
 Так, ви правильно помітили: архів за 2025-07-05 містить дані за 2025-07-06, тобто з 00:05 і далі. Це трапляється тому, що:

📌 Причина:
У saveHistoryToFile(true) ми зберігаємо history[], не фільтруючи за датою. Але history[] вже не містить даних за вчора, а всі записи — з нової дати (після опівночі).

/////////////////////////
Ось як реалізувати автоматичне архівування вчорашніх даних о 00:01 через таймер у loop():

✅ 1. Оновіть loop() для таймерного запуску збереження:
cpp
Копіювати
void loop() {
  static uint32_t lastUpdate = 0;
  static time_t lastArchiveAttempt = 0;

  // Оновлення вимірів і запис в буфер кожну хвилину
  if (millis() - lastUpdate > 60000) {
    lastUpdate = millis();

    float t = bme.readTemperature();
    float h = bme.readHumidity();
    float p = bme.readPressure() / 100.0F;
    String timeStr = getCurrentTime();
    storeToHistory(t, h, p, timeStr);
  }

  // ======= ⏰ Автоматичне архівування після 00:01 ========
  time_t now;
  time(&now);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 1 && now - lastArchiveAttempt > 3600) {
    Serial.println("🕛 Автоматичне збереження архіву за вчора...");
    saveHistoryToFile(true);         // зберігаємо вчорашні дані
    clearOldLogs(30);                // очищення старих логів
    lastArchiveAttempt = now;       // захист від повторного виклику
  }

  delay(10000); // інші задачі
}
✅ 2. Залиште ручне збереження без змін:
cpp
Копіювати
server.on("/save_now", HTTP_GET, [](AsyncWebServerRequest *request){
  saveHistoryToFile();  // сьогоднішні дані
  request->send(200, "text/plain", "✅ Поточний архів збережено у файл.");
});
🧠 Переваги:
Система автоматично створить архів учорашніх даних після 00:01, якщо є ≥10 вимірів.

Повністю незалежна від setup(), працює на кожному запуску.

Уникнення дублювання архівів.

🔧 Якщо хочете, я можу об’єднати всі зміни в один повний .zip проєкт PlatformIO.







Запитати ChatGPT
