#include "text_data.h"

TextLine poem1[] = {
  { "Як умру, то поховайте",  0,  0,   TFT_WHITE, TFT_BLUE },
  { "Мене на могилі,",       10, 22,   TFT_WHITE, TFT_BLUE },
  { "Серед степу широкого,",  0, 44,   TFT_BLACK, TFT_YELLOW },
  { "На Вкраїні милій.",     10, 66,   TFT_BLACK, TFT_YELLOW },
};

size_t poem1Size = sizeof(poem1) / sizeof(poem1[0]);

TextLine poem2[] = {
  { "Щоб лани широкополі",  0,  0,   TFT_WHITE, TFT_BLUE },
  { "І Дніпро, і кручі",    10, 22,   TFT_WHITE, TFT_BLUE },
  { "Було видно, було чути", 0, 44,   TFT_BLACK, TFT_YELLOW },
  { "Як реве ревучий.",     10, 66,   TFT_BLACK, TFT_YELLOW },
};

size_t poem2Size = sizeof(poem2) / sizeof(poem2[0]);
