#include "Set_date.h"
#include "globals.h"
//#include "RTClib.h"
//int8_t  day;

void set_real_date (void)// 
                       
 {
    unsigned char t=1;

    unsigned char m=1;

 unsigned char f_enter = 0;
 unsigned char f_plus = 0;
 unsigned char f_minus = 0;
 DateTime now = rtc.now();//

  day = now.day();

    while( m )
    {
        f_plus = button(PLUS);
        f_minus = button(MINUS);
        f_enter = f_enter + button(UVOD);


          if (!f_enter)
          {                   
                               
          day = day + f_plus;
          day = day - f_minus;
          f_plus =0; 
          f_minus = 0;
                    if (day > 31) day = 1;
                    if (day < 1) day = 31;
					
  //tft.setRotation(3);//Альбомная орієнтація
  
 tft.fillScreen(TFT_BLACK);    		
  tft.setCursor(10, 40);//Встановити курсор в позицію Х У
  tft.setTextColor(TFT_GREEN);//Колір шрифта
  //tft.setTextSize(2);//Розмір шрифта "2" 
  if(day<10)
  {
   tft.print(0);
   tft.print(day); } 
  else
   {tft.print(day);
  } 		
					

          }
                             
        if (f_enter) 
        {
        //set_Data.date = ds3231_perev_for_ds(t);
        f_enter=0;
        f_plus =0; 
        f_minus = 0;
        m =0;
        }
         
    delay(500);
	
        }

 }