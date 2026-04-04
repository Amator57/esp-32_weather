// --- Константи ---
const int MAX_MEASUREMENTS = 1440;  // 24 години * 60 хв
const char* LOG_DIR = "/log";

// --- Структура даних ---
struct BMEData {
  float temperature;
  float humidity;
  float pressure;
  String timeStr;
};

BMEData history[MAX_MEASUREMENTS];
int historyIndex = 0;

// --- Функція збереження в RAM ---
void storeToHistory(float t, float h, float p, String timeStr) {
  history[historyIndex] = { t, h, p, timeStr };
  historyIndex = (historyIndex + 1) % MAX_MEASUREMENTS;
}

// --- Отримати сьогоднішню дату у форматі YYYY-MM-DD ---
String getTodayDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "unknown";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
  return String(buf);
}

// --- Зберегти поточні 1440 записів у файл SPIFFS ---
void saveHistoryToFile() {
  String filename = String(LOG_DIR) + "/" + getTodayDate() + ".csv";
  File f = SPIFFS.open(filename, FILE_WRITE);
  if (!f) {
    Serial.println("[ERR] Не вдалося створити лог-файл");
    return;
  }
  f.println("Час,Температура,Вологість,Тиск");
  for (int i = 0; i < MAX_MEASUREMENTS; i++) {
    int idx = (historyIndex + i) % MAX_MEASUREMENTS;
    BMEData& d = history[idx];
    f.printf("%s,%.2f,%.2f,%.2f\n", d.timeStr.c_str(), d.temperature, d.humidity, d.pressure);
  }
  f.close();
  Serial.println("[OK] Збережено: " + filename);
}

// --- Видалити лог-файли старші за N днів ---
void clearOldLogs(int maxDays) {
  File root = SPIFFS.open(LOG_DIR);
  if (!root || !root.isDirectory()) return;

  time_t now;
  time(&now);

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    file.close();

    if (name.endsWith(".csv")) {
      name.replace(String(LOG_DIR) + "/", "");
      struct tm tm = {};
      if (strptime(name.c_str(), "%Y-%m-%d.csv", &tm)) {
        time_t fileTime = mktime(&tm);
        double ageDays = difftime(now, fileTime) / 86400.0;
        if (ageDays > maxDays) {
          String fullpath = String(LOG_DIR) + "/" + name;
          SPIFFS.remove(fullpath);
          Serial.println("[DEL] Старий файл: " + fullpath);
        }
      }
    }
    file = root.openNextFile();
  }
}
