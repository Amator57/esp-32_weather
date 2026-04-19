# Оптимізація Метеостанції (Implementation Plan)

Цей план містить технічні деталі впровадження всіх чотирьох пунктів оптимізації, описаних у `system_review_and_manual.md`.

## User Review Required
> [!IMPORTANT]
> Будь ласка, перегляньте цей план. Зміни зачіпають значну частину файлу `main.cpp` та `subroutines.cpp`. Після вашого схвалення я почну вносити зміни у вказані файли.

## Proposed Changes

---

### Ядро системи та Пам'ять (Core & Memory)

#### [MODIFY] [globals.h](file:///C:/Projects_PlatformIO/Final_ESP32__Web_weather%20station/ESP32__Web_weather%20station_V2026/include/globals.h)
- Змінити `String timeStr` на `char timeStr[20]` у структурі `BMEData`.
- Додати глобальні змінні для Кільцевого буфера: `extern uint32_t totalMeasurements;` та `extern uint32_t lastSavedTotal;`. 

#### [MODIFY] [main.cpp](file:///C:/Projects_PlatformIO/Final_ESP32__Web_weather%20station/ESP32__Web_weather%20station_V2026/src/main.cpp)
- **Оптимізація масиву Історії ($O(1)$ замість $O(N)$)**: 
  - Переписати функцію `storeToHistory()` або `saveCurrentReadingToHistory()`, використовуючи збільшення `totalMeasurements` та індексацію за залишком від ділення (`totalMeasurements % MAX_MEASUREMENTS`).
  - Переписати функцію збереження `saveHistoryTask()`, щоб вона ітерувала від `lastSavedTotal` до `totalMeasurements`.
- **Файлова система та Strings**:
  - Переписати `clearOldLogs()`: замість постійного конструювання об'єктів `String`, використовувати статичні масиви (`char fileNameBuf[32]`) для обходу директорії `SPIFFS`.

---

### Вебсервер (Web Handlers)

#### [MODIFY] [main.cpp](file:///C:/Projects_PlatformIO/Final_ESP32__Web_weather%20station/ESP32__Web_weather%20station_V2026/src/main.cpp)
- Видалити величезний блок дубльованих роутів `server.on(..., [](...) { ... })` із функції `setup()` (приблизно 100-150 рядків).
- Додати єдиний виклик `registerWebHandlers(server);` одразу після ініціалізації Wi-Fi та SPIFFS. Це підтягне всі роути з існуючого оптимізованого файлу `web_handlers.h`.

---

### Користувацький Інтерфейс (TFT Display)

#### [MODIFY] [subroutines.cpp](file:///C:/Projects_PlatformIO/Final_ESP32__Web_weather%20station/ESP32__Web_weather%20station_V2026/src/subroutines.cpp)
- **Усунення блокувань**: Функції `shevchenko()`, `vivat()` та `slava_ukraini()` використовують жорсткі `delay(5000)` та `delay(1000)`. Замінити їх на виклик `vTaskDelay(5000 / portTICK_PERIOD_MS)` з примусовою передачею управління системі (yield), або переписати на неблокуючі таймери з використанням `millis()`.

## Open Questions
- Чи використовуєте ви зараз збереження `history[]` ще десь у коді крім `/bme_chart_data` та SPIFFS логера? Перехід на `char[20]` і кільцевий буфер вимагатиме суворого форматування. 

## Verification Plan
### Automated Tests
- Запуск `platformio run` для перевірки відсутності помилок компіляції після комплексного рефакторингу.
### Manual Verification
- Перевірка підключення до Wi-Fi, відображення екрану та працездатності Web Server-а після заливки оновленого коду на плату.
