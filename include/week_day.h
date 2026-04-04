#pragma once

void set_real_weekday();
void week_day_out (unsigned char x);









//#include "utf8rus.h"
 //Test
//#include "Fonts/GFXFF/FreeSerif9pt7b.h" // Підключення кириличного шрифту
//#include "globals.h"
//#include <Arduino.h>
//#include "TextLine.h" 
//#include "printng_Line.h"
//

/*
void vivat(void) {
// вивід вітання
        int16_t screenWidth = tft.width();
        int16_t screenHeight = tft.height();
        
          tft.setRotation(1);//Альбомная орієнтація
          tft.setTextWrap(false);
          tft.fillRect(0, 0, screenWidth, screenHeight / 2, ST7735_BLUE);//Зафарбувати верхню половину екрану в синій колір
        
          tft.fillRect(0, screenHeight / 2, screenWidth, screenHeight / 2, ST7735_YELLOW);//Зафарбувати нижню половину екрану в жовтий колір
//tft.setFont(&FreeSans9pt7b); // Використовуйте setFont замість setTextFont

        
          //tft.fillScreen(ST7735_BLUE);//Очистить дисплей“
          //tft.setCursor(10, 0);//Поставить курсор Х У
       tft.setTextSize(3);//Размер шрифта "3"
    tft.setTextColor(ST7735_YELLOW);//Цвет шрифта 
       
    tft.drawString("Слава", 10, 0); //неділя
    //tft.setFont(&FreeSerif9pt7b);
     //tft.setCursor(20,35);//Поставить курсор Х 
     tft.drawString("Украынi!", 20, 35); //неділя
 //Test   
  //tft.println("Українi!"); //неділя 

    
      //tft.setTextWrap(false);
      //tft.fillScreen(ST7735_BLUE);//Очистить дисплей“
    
    //tft.setCursor(10,70);//Поставить курсор Х У  
       tft.setTextSize(3);//Размер шрифта "3"
    tft.setTextColor(ST7735_BLUE);//Цвет шрифта 
    
    tft.drawString("Героям", 10, 70); //неділя
    //tft.setCursor(30,100);//Поставить курсор Х У
    tft.drawString("Слава!", 30, 100); //
    //delay(15000);
    // tft.fillScreen(ST7735_BLACK);//Очистить дисплей
    // вивід вітання
}

void zapovit(void)
{
   
   tft.setTextWrap(true);
   tft.fillScreen(ST7735_BLACK);//Очистить дисплей“
   tft.setTextSize(2);//Размер шрифта "1"
   tft.setTextColor(ST7735_WHITE);//Цвет шрифта 
   //tft.setCursor(10,30);//Поставить курсор Х У
   //tft.println(utf8rus("Щоб лани широкополi\n i Днiпро i кручi \nстали вам поперек горла\n москалi ыбучi")); //
   tft.drawString("Щоб лани\n  широкополi\n    i Днiпро\n     i кручi", 10, 30); //
   delay(5000);
   tft.fillScreen(ST7735_BLACK);//Очистить дисплей“
   tft.setTextSize(2);//Размер шрифта "1"
   tft.setTextColor(ST7735_RED);//Цвет шрифта 
   //tft.setCursor(10,30);//Поставить курсор Х У
   tft.drawString("стали вам\n  поперек\n   горла\n    москалi\n     ыбучi", 10, 30); 

    delay(15000);
}

void rus_ship(void)
{



}
*/
//
/*
void will(void)
{


   // Синій верх
tft.fillRect(0, 0, screenW, screenH / 2, TFT_BLUE);
// Жовтий низ
tft.fillRect(0, screenH / 2, screenW, screenH / 2, TFT_YELLOW);

// Виведення першого блоку з індивідуальними координатами
for (int i = 0; i < 3; i++) {
  printLineByGrow(textBlock1, i, xBlock1[i], yBlock1[i], 160, TFT_WHITE, TFT_BLUE);//текст, індекс, x, y, ширина, колір тексту, колір фону 
}

// Пауза
delay(1000);

// Виведення другого блоку з іншими координатами
for (int i = 0; i < 3; i++) {
  printLineByGrow(textBlock2, i, xBlock2[i], yBlock2[i], 160, TFT_BLACK, TFT_YELLOW);//текст, індекс, x, y, ширина, колір тексту, колір фону 
} 

}
*/