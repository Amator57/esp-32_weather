#include "globals.h"
#include "text_effects.h"
/*
struct TextLine {
    const char* text;       // Рядок тексту
    int x;                  // Початкова координата X
    int y;                  // Початкова координата Y
    uint16_t textColor;     // Колір тексту
    uint16_t bgColor;       // Колір фону
    int delayMs;            // Затримка між символами
  };
*/
/*
TextLine lines7[] = {
    { "Щоб  лани",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
    { "широкополі",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
    { "і  Дніпро,  і  кручі", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
    { "Стали  вам",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
    { "поперек  горла", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
    { "москалі   їбучі", 25, 110, TFT_BLACK, TFT_YELLOW, 100 },
  };
    //////////////////////////////////////////////////////////////////////////////////
 
    TextLine lines1[] = {
        { "Як умру, то ",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Поховайте",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "Мене на могилі", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Серед степу ",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Широкого", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "На Вкраїні милій,", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };

      TextLine lines2[] = {
        { "Щоб лани  ",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Широкополі,",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "І Дніпро, і кручі", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Було видно,",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Було чути,", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Як реве ревучий.", 25, 110, TFT_BLACK, TFT_YELLOW, 100 },
      };
 
      TextLine lines3[] = {
        { "Як понесе з України",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "У синєє море",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "Кров ворожу... ", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Отойді я i лани i",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Гори - все покину,", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "І полину", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };
 
      TextLine lines4[] = {
        { "До самого Бога",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Молитися...",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "А до того ", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Я не знаю Бога.",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Поховайте та ", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Вставайте,", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };

      TextLine lines5[] = {
        { "Кайдани порвіте i",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Вражою злою",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "Кров'ю волю ", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Окропіте. ",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "І мене в сім'ї  ", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Великій, в сім'ї", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };

      TextLine lines6[] = {
        { "Вольній, новій,",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Не забудьте",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "Пом'янути", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Незлим тихим ",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Словом.", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Т.Г. Шевченко", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };



           TextLine lines8[] = {
        { "СЛАВА",      5,  20,   TFT_WHITE, TFT_BLUE, 100 },
        { "УКРАЇНІ!",     45,  90,  TFT_BLACK, TFT_YELLOW, 100 }
     };

   TextLine lines9[] = {
        { "Героям",       5,  20,   TFT_WHITE, TFT_BLUE, 100 },
        { "Слава!",     45,  90,  TFT_BLACK, TFT_YELLOW, 100 }
      };   
*/

TextLine lines7[] = {
    { "Щоб  лани",       45,  10,   TFT_WHITE, TFT_BLUE, 100 },
    { "широкополі",     55, 45,  TFT_WHITE, TFT_BLUE, 100 },
    { "і  Дніпро,  і  кручі", 65, 80,  TFT_WHITE, TFT_BLUE, 100 },
    { "Стали  вам",     45, 130,    TFT_BLACK, TFT_YELLOW, 100 },
    { "поперек  горла", 55,  165,  TFT_BLACK, TFT_YELLOW, 100 },
    { "москалі   їбучі", 65, 200, TFT_BLACK, TFT_YELLOW, 100 },
  };
    //////////////////////////////////////////////////////////////////////////////////
 
    TextLine lines1[] = {
        { "Як умру, то поховайте ",  20,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Мене на могилі,",     30,  30,  TFT_WHITE, TFT_BLUE, 100 },
        { "Серед степу шикорого,", 40, 60,  TFT_WHITE, TFT_BLUE, 100 },
        { "На Вкраїні милій, ",   50,  90,  TFT_WHITE, TFT_BLUE, 100 },
        { "Щоб лани широкополі",  20,  125,  TFT_BLACK, TFT_YELLOW, 100 },
        { "І Дніпро, і кручі", 30, 155, TFT_BLACK, TFT_YELLOW, 100 },
        { "Було видно, було чути,", 40, 185, TFT_BLACK, TFT_YELLOW, 100 },
        { "Як реве ревучий.", 50, 215, TFT_BLACK, TFT_YELLOW, 100 }
      };
/*
      TextLine lines2[] = {
        { "Щоб лани широкополі,  ",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "І Дніпро, і кручі", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { "Було видно,",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Було чути,", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Як реве ревучий.", 25, 110, TFT_BLACK, TFT_YELLOW, 100 },
      };
 */
      TextLine lines3[] = {
        { "Як понесе з України", 20,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "У синєє море",     30,  30,  TFT_WHITE, TFT_BLUE, 100 },
        { "Кров ворожу... отойді я", 40, 60,  TFT_WHITE, TFT_BLUE, 100 },
        { "І лани i гори -",     50,  90,  TFT_WHITE, TFT_BLUE, 100 },
        { "Все покину, і полину", 20,  125,  TFT_BLACK, TFT_YELLOW, 100 },
        { "До самого Бога", 30, 155, TFT_BLACK, TFT_YELLOW, 100 },
        { "Молитися...А до того -", 40, 185, TFT_BLACK, TFT_YELLOW, 100 },
        { "Я незнаю Бога.", 50, 215, TFT_BLACK, TFT_YELLOW, 100 }
      };
 
      TextLine lines4[] = {
        { "Поховайте та вставайте.", 20,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "Кайдани порвіте",     30,  30,  TFT_WHITE, TFT_BLUE, 100 },
        { "І вражою злою кров'ю", 40, 60,  TFT_WHITE, TFT_BLUE, 100 },
        { "Волю окропіте.",     50,  90,  TFT_WHITE, TFT_BLUE, 100 },
        { "І мене в сім'ї великій,", 20,  125,  TFT_BLACK, TFT_YELLOW, 100 },
        { "В сім'ї вольній, новій", 30,  155,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Не забудьте пом'янути", 40, 185,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Незлим тихим словом.", 50, 215, TFT_BLACK, TFT_YELLOW, 100 }
      };
/*
      TextLine lines5[] = {
        { "",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "  ", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { " ",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "  ", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { ", ", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };

      TextLine lines6[] = {
        { ",",       5,  0,   TFT_WHITE, TFT_BLUE, 100 },
        { "",     15,  22,  TFT_WHITE, TFT_BLUE, 100 },
        { "", 25, 44,  TFT_WHITE, TFT_BLUE, 100 },
        { " ",     5,  66,  TFT_BLACK, TFT_YELLOW, 100 },
        { "", 15,  88,  TFT_BLACK, TFT_YELLOW, 100 },
        { "Т.Г. Шевченко", 25, 110, TFT_BLACK, TFT_YELLOW, 100 }
      };
*/


           TextLine lines8[] = {
        { "СЛАВА",      40,  40,   TFT_WHITE, TFT_BLUE, 100 },
        { "УКРАЇНІ!",     100,  160,  TFT_BLACK, TFT_YELLOW, 100 }
     };

   TextLine lines9[] = {
        { "Героям",       40,  40,   TFT_WHITE, TFT_BLUE, 100 },
        { "Слава!",    100,  160,  TFT_BLACK, TFT_YELLOW, 100 }
      };   

void shevchenko (void)// Дисплей
{
 tft.loadFont(My_ariali_20); 
 // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
 for (int i = 0; i < sizeof(lines1) / sizeof(lines1[0]); i++) {
    printLineTypingEffect(lines1[i], screenW, lineHeight);
 }
    delay(1000); // Затримка між рядками (можна налаштувати)
/*
// Синій верх
tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
// Жовтий низ
tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
for (int i = 0; i < sizeof(lines2) / sizeof(lines2[0]); i++) {
  printLineTypingEffect(lines2[i], screenW, lineHeight);
}
  delay(1000); // Затримка між рядками (можна налаштувати)
*/
   // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
 for (int i = 0; i < sizeof(lines3) / sizeof(lines3[0]); i++) {
  printLineTypingEffect(lines3[i], screenW, lineHeight);
 }
  delay(1000); // Затримка між рядками (можна налаштувати)

   // Синій верх
   tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
   // Жовтий низ
   tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
   for (int i = 0; i < sizeof(lines4) / sizeof(lines4[0]); i++) {
    printLineTypingEffect(lines4[i], screenW, lineHeight);
   }
    delay(1000); // Затримка між рядками (можна налаштувати)
    tft.unloadFont();
/*
       // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
 for (int i = 0; i < sizeof(lines5) / sizeof(lines5[0]); i++) {
  printLineTypingEffect(lines5[i], screenW, lineHeight);
 }
  delay(1000); // Затримка між рядками (можна налаштувати)

    // Синій верх
    tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
    // Жовтий низ
    tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
    for (int i = 0; i < sizeof(lines6) / sizeof(lines6[0]); i++) {
     printLineTypingEffect(lines6[i], screenW, lineHeight);
    }
     delay(5000); // Затримка між рядками (можна налаштувати)
     */
     /*
        // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
 for (int i = 0; i < sizeof(lines7) / sizeof(lines7[0]); i++) {
  printLineTypingEffect(lines7[i], screenW, lineHeight);
 }
  delay(5000); // Затримка між рядками (можна налаштувати)
  */
 }


void vivat(void) //москалі..
{
  tft.loadFont(My_ariali_26);
  // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
 for (int i = 0; i < sizeof(lines7) / sizeof(lines7[0]); i++) {
  printLineTypingEffect(lines7[i], screenW, lineHeight);
 }
  delay(5000); // Затримка між рядками (можна налаштувати) 
  tft.unloadFont();
}



void slava_ukraini(void)
{
  tft.loadFont(My_ariali_34);
  // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);


 for (int i = 0; i < sizeof(lines8) / sizeof(lines8[0]); i++) {
  printLineTypingEffect(lines8[i], screenW, lineHeight);
 }
  delay(5000); // Затримка між рядками (можна налаштувати) 

  // Синій верх
 tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
 // Жовтий низ
 tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
for (int i = 0; i < sizeof(lines9) / sizeof(lines9[0]); i++) {
  printLineTypingEffect(lines9[i], screenW, lineHeight);
 }
  delay(5000); // Затримка між рядками (можна налаштувати)
tft.unloadFont();
}



// ... (ваші інші функції, такі як displayDateTime, week_day_out, shevchenko, vivat, slava_ukraini, printLineTypingEffect)
/*
// Нова функція для відображення даних BME280
void displayBMEData() {
    tft.fillScreen(TFT_BLACK); // Повністю очищаємо екран
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Встановлюємо колір тексту за замовчуванням

    // Заголовок
    tft.loadFont(My_ariali_24); // Використовуємо більший шрифт для заголовка
    tft.setCursor(10, 10);
    tft.print("BME280 Дані:");

    // Температура
    tft.loadFont(My_ariali_28); // Шрифт для значень
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 50);
    tft.printf("Темп: %.1f C", lastTemp); // Використовуємо lastTemp

    // Вологість
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.setCursor(10, 100);
    tft.printf("Волог: %.1f %%", lastHum); // Використовуємо lastHum

    // Тиск
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(10, 150);
    tft.printf("Тиск: %.1f мм.рт.ст.", lastPress / 133.322); // Конвертуємо Па в мм.рт.ст.
    tft.setCursor(10, 180); // Додатковий рядок для Па
    tft.printf("      %.1f Па", lastPress); // Використовуємо lastPress
}
*/
// ... (кінець subroutines.cpp)

/*
void shevchenko() {
    tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
    tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);
    tft.loadFont(My_arial_14);

    for (auto& line : lines1)
        printLineTypingEffect(line, screenW, lineHeight);

    delay(1000);

    tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
    tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);

    for (auto& line : lines2)
        printLineTypingEffect(line, screenW, lineHeight);

    tft.unloadFont();
    delay(1000);
}

void welcomeMessage() {
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(My_arial_14);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(10, 50);
    tft.println("Вітаємо!");
    tft.unloadFont();
    delay(2000);
}
*/