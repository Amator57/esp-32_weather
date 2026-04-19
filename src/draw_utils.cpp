#include "globals.h"
#include "draw_utils.h"

void drawPoem(TextLine* lines, size_t count) {
  // Очистка фону
  tft.fillScreen(TFT_BLACK);

  for (size_t i = 0; i < count; ++i) {
    tft.setTextColor(lines[i].color, lines[i].bg);
    tft.drawString(lines[i].text, lines[i].x, lines[i].y);
    vTaskDelay(500 / portTICK_PERIOD_MS);  // "ефект поступового з'явлення"
  }
}
