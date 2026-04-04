#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

struct TextLine {
  const char* text;
  int x;
  int y;
  uint16_t color;
  uint16_t bg;
};

extern TextLine poem1[];
extern size_t poem1Size;

extern TextLine poem2[];
extern size_t poem2Size;
