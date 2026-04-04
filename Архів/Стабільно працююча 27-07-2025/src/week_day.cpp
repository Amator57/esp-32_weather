#include "week_day.h"
#include "globals.h" // якщо тут є tft, rtc тощо

// Реалізація week_day_out() (наприклад, у файлі week_day.h або subroutines.cpp)
void week_day_out (unsigned char x) {
    // Шрифт вже завантажений executeDisplayMode() при вході в MODE_DATETIME.
    // === ВАЖЛИВО: Видаліть tft.loadFont() та tft.unloadFont() звідси! ===

    tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Колір тексту для дня тижня

    tft.fillRect(40, 155, 200, 30, TFT_BLACK); // Очищуємо область дня тижня
    switch (x) {
        case 0: tft.drawString("Недiля", 70, 175); break;
        case 1: tft.drawString("Понедiлок", 70, 175); break;
        case 2: tft.drawString("Вiвторок", 70, 175); break;
        case 3: tft.drawString("Середа", 70, 175); break;
        case 4: tft.drawString("Четвер", 70, 175); break;
        case 5: tft.drawString("Ура!   П'ятниця!", 50, 175); break;
        case 6: tft.drawString("Субота", 70, 175); break;
    }
}

/*
// Реалізація week_day_out() (наприклад, у файлі week_day.h або subroutines.cpp)
void week_day_out (unsigned char x) {
    // === ВАЖЛИВО: Видаліть tft.loadFont() та tft.unloadFont() звідси! ===
    // Шрифт вже завантажений у displayDateTime().

    // Встановлюємо колір тексту для дня тижня (як ви хотіли - жовтий)
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // Очищуємо область, де відображається день тижня, перед малюванням нового тексту.
    // Ширина 200 пікселів та висота 30 пікселів - це приклад, можливо, потрібно адаптувати
    // під найдовший можливий рядок ("Ура!   П'ятниця!").
    tft.fillRect(40, 155, 200, 30, TFT_BLACK); 

    switch (x) {
        case 0: tft.drawString("Недiля", 40, 155); break;
        case 1: tft.drawString("Понедiлок", 40, 155); break;
        case 2: tft.drawString("Вiвторок", 40, 155); break;
        case 3: tft.drawString("Середа", 40, 155); break;
        case 4: tft.drawString("Четвер", 40, 155); break;
        case 5: tft.drawString("Ура!   П'ятниця!", 40, 155); break;
        case 6: tft.drawString("Субота", 40, 155); break;
    }
    // tft.unloadFont(); // === ВАЖЛИВО: Видаліть цей рядок! ===
}
*/
/*
void week_day_out (unsigned char x)
{
    tft.loadFont(My_ariali_26);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                 switch (x)
                 {
                 case 0:
                 tft.drawString("Недiля", 40, 155); //неділя
                  
                 break;
                 case 1:
                 tft.drawString("Понедiлок", 40, 155);//понеділок
                 
                 break;
                 case 2:
                 tft.drawString("Вiвторок", 40, 155); //вівторок
                 
                 break;
                 case 3:
                 tft.drawString("Середа", 40, 155); //середа
                 
                 break;
                 case 4:
                 tft.drawString("Четвер", 40, 155); //четвер
                 
                 break;
                 case 5:
                 tft.drawString("Ура!   П'ятниця!", 40, 155); //п"ятниця
                 
                 break;
                 case 6:
                 tft.drawString("Субота", 40, 155); //субота
                 
                 break;
                 } 
     tft.unloadFont();             
}
*/