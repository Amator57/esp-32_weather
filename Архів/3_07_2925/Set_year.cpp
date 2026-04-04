#include "Set_year.h"
#include "globals.h" // якщо тут є tft, rtc тощо

void set_real_year (void)// 
                       
 {
    //unsigned char t=25;
    
    unsigned char m=1;

 unsigned char f_enter = 0;
 unsigned char f_plus = 0;
 unsigned char f_minus = 0;
 DateTime now = rtc.now();
 year = now.year();
 
    while( m )
    {
        f_plus = button(PLUS);
        f_minus = button(MINUS);
        f_enter = f_enter + button(UVOD);


          if (!f_enter)
          {                   
                               
          year = year + f_plus;
          year = year - f_minus;
          f_plus =0;
          f_minus = 0;
                    if (year >= 99) year = 99;
                    if (year <= 25) year = 25; 

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_BLUE);//Колір шрифта
  //tft.setTextSize(2);//Розмір шрифта "2"           
  tft.setCursor(10, 40);//Встановити курсор в позицію Х У
  //tft.setTextColor(TFT_BLUE);//Колір шрифта
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
		
  } 												
					                                       
        if (f_enter) 
        {
        f_enter=0;
        f_plus =0; 
        f_minus = 0;
        m =0;
        }
        
     delay(500);   

 }

 }