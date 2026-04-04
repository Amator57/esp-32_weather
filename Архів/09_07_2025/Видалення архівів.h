//Автоматичне видалення архівів, старших за X днів
//Це дозволить зберігати, наприклад, лише останні 30 днів логів.

//✅ Код для функції clearOldLogs(int daysToKeep)
//cpp

void clearOldLogs(int daysToKeep) {
  File root = SPIFFS.open("/log");
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Не вдалося відкрити директорію /log для очищення");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = file.name();  // /log/2025-06-30.csv

    // Витягуємо YYYY-MM-DD
    if (name.startsWith("/log/") && name.endsWith(".csv") && name.length() == 20) {
      String dateStr = name.substring(5, 15);  // "2025-06-30"

      // Перевірка формату
      int year = dateStr.substring(0, 4).toInt();
      int month = dateStr.substring(5, 7).toInt();
      int day = dateStr.substring(8, 10).toInt();

      struct tm fileDate = {0};
      fileDate.tm_year = year - 1900;
      fileDate.tm_mon = month - 1;
      fileDate.tm_mday = day;

      time_t fileTime = mktime(&fileDate);
      time_t now;
      time(&now);

      double ageInDays = difftime(now, fileTime) / 86400.0;

      if (ageInDays > daysToKeep) {
        Serial.printf("🗑 Видаляю файл %s (%d днів тому)\n", name.c_str(), (int)ageInDays);
        SPIFFS.remove(name);
      }
    }

    file = root.openNextFile();
  }
}

//📍 Коли її викликати?
//В кінці saveHistoryTask() (після очищення history[]):

//cpp

clearOldLogs(30);  // наприклад, зберігати лише останні 30 днів
/*
✅ Підсумок
Компонент	Статус
Архівування — фонове	✅ так, xTaskCreatePinnedToCore(...)
Очищення history[] — після запису	✅
Створення файлу о 00:01	✅
Видалення архівів > X днів	🔧 додано через clearOldLogs()

🔧 Хочете — я можу оновити весь файл loop_and_saveHistoryTask.cpp з уже вбудованою clearOldLogs(30) та повною інтеграцією?
*/









