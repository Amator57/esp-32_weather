#include "globals.h"
#include "text_effects.h"
/*
void printLineTypingEffect(TextLine line, int screenW, int lineHeight) {
    tft.setTextColor(line.textColor, line.bgColor);
    tft.setCursor(line.x, line.y);
    for (size_t i = 0; i < strlen(line.text); ++i) {
        tft.print(line.text[i]);
        delay(line.delayMs / strlen(line.text));
    }
}
*/
//////////////////Найкращий результат/////////////////////// 
void printLineTypingEffect(const TextLine& line, int screenW, int lineHeight) {
  // Очистити область перед виведенням тексту (одноразово)
  tft.fillRect(0, line.y, screenW, lineHeight, line.bgColor);

  tft.setCursor(line.x, line.y);                            // Встановлюємо курсор
  tft.setTextColor(line.textColor, line.bgColor);           // Встановлюємо кольори

  for (int i = 0; i < strlen(line.text); i++) {
    tft.print(line.text[i]);                                // Друкуємо символ
    vTaskDelay(line.delayMs / portTICK_PERIOD_MS);          // Затримка між символами
  }
}

//Виклик функції:
//for (int i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
//  printLineTypingEffect(lines[i], screenW, lineHeight);
//}