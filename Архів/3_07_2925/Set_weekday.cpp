#include "Set_weekday.h"
#include "globals.h" // якщо тут є tft, rtc тощо




void set_real_weekday (void)                     
 {
 
/*
  unsigned char m=1;
  unsigned char f_enter = 0;
  unsigned char f_plus = 0;
  unsigned char f_minus = 0;
  unsigned char f_long = 0;
  unsigned char week = 0;
*/
 DateTime now = rtc.now();

 week=now.dayOfTheWeek();
//week = watch.weekday;
                          
    while( m )
    {
        f_plus = button(PLUS);
        f_minus = button(MINUS);
        f_enter = f_enter + button(UVOD);
        f_long = long_button(UVOD); 
          week = week + f_plus;
          week = week - f_minus;

                    if (week > 6) week = 0;
                    if (week < 0) week = 6; 
                    

                                        f_plus =0; 
                                        f_minus = 0;
                  
                                       
                                       
  tft.setRotation(3);//Альбомная орієнтація
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);//Очистить дисплей“                    
                   
                tft.setCursor(10, 40);//Встановити курсор в позицію Х У
  tft.setTextColor(TFT_BLUE);//Колір шрифта
  //tft.setTextSize(2);//Розмір шрифта "2"
  if(day<10)
  {
   tft.print(0);
   tft.print(day); } 
  else
   {tft.print(day);
  } 
tft.println(":");

  tft.setCursor(45, 40);//Поставить курсор Х У
  if(month<10)
  {
   tft.print(0);
   tft.print(month); } 
  else
   {tft.print(month);
  }  
   
tft.println(":");
 tft.setCursor(80, 40);//Поставить курсор Х У
   tft.print(20);
   tft.print(year);    

  tft.setCursor(10, 80);//Поставить курсор Х У
  tft.setTextColor(TFT_GREEN);//Цвет шрифта 
  //tft.setTextSize(2);//Размер шрифта "2"
week_day_out(week);     
                                        
 delay(1000);        
      f_enter = uvod();               
      //if (f_long)
      if (f_enter) 
        {
        f_enter = 0;
        f_plus = 0; 
        f_minus = 0;
        m =0;
        }
                   
 }
  
 }