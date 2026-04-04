#pragma once
struct TextLine {
    const char* text;
    int x;
    int y;
    uint16_t textColor;
    uint16_t bgColor;
    int delayMs;
};

//void printLineTypingEffect(TextLine line, int screenW, int lineHeight);
void printLineTypingEffect(const TextLine& line, int x, int y);