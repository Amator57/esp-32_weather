#include "Set_hour.h"
#include "globals.h"

//#include "RTClib.h"
void set_real_hour (void)// 
                       
 {
  int8_t  week;
  int16_t year;
  int8_t  month;
    signed char hour=12;
    signed char minute=30;
    signed char second=30;
  
 unsigned char n=1;

 unsigned char  f_long = 0;
 unsigned char f_enter = 0;
 unsigned char f_plus = 0;
 unsigned char f_minus = 0;
 DateTime now = rtc.now();
 hour = now.hour();
 minute = now.minute();
// second = watch.seconds;
 
    while( n )
    {
        f_plus = button(PLUS);
        f_minus = button(MINUS);
        //
        f_enter = f_enter + button(UVOD);
        f_long =  long_button(UVOD);
          
          if (!f_enter)
          {                   
                    
         hour = hour + f_plus;
         hour = hour - f_minus;
          f_plus =0; 
          f_minus = 0;
                    if (hour >23) hour = 0;
                    if (hour < 0) hour = 23;
      // f_enter = f_enter + button(UVOD);             
		  }		
					  

 if (f_enter == 1)
          {                                     
          minute = minute + f_plus;
          minute = minute - f_minus;
          f_plus =0; 
          f_minus = 0;
                    if (minute >59) minute = 0;
                    if (minute < 0) minute = 59;
       //f_enter = f_enter + button(UVOD);
		  }

         if (f_enter >= 2) 
        {
        second=0;
        //watch.settime(second, minute, hour, day, month, year, week);
        rtc.adjust(DateTime(year, month, day, hour, minute, 0)); 
        f_enter=0;
        f_plus =0; 
        f_minus = 0;
        f_long=0;
        n =0;
        }         			



					
//Індикація

  tft.setRotation(3);//Альбомная орієнтація
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);//Очистить дисплей“   
//Години
  tft.setCursor(30, 0);//Поставить курсор Х У
  tft.setTextColor(TFT_RED);//Колір шрифта
  //tft.setTextSize(3);//Размер шрифта "3"
  if(hour<10)
  {
   tft.print(0);
   tft.print(hour);} 
  else
   {
    tft.print(hour);
   }

  tft.println(":");
//Хвилини
if (f_enter >= 1)
{
  tft.setCursor(82, 0);
  if(minute<10)
  {
   tft.print(0);
   tft.print(minute); } 
  else
   {tft.print(minute);
  }       
//  tft.println(":");
}
//Секунди (не встановлюються)
// if (f_enter >= 2)
 //{
  // tft.setCursor(115, 0);
 /*  
  if(second<10)
  {
   tft.print(0);
   tft.print(second); } 
  else
   {tft.print(second);
  }
  */   
  // tft.print(0);
  // tft.print(0);
// } 
 //Дата             
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
//Місяць
  tft.setCursor(45, 40);//Поставить курсор Х У
  if(month <10)
  {
   tft.print(0);
   tft.print(month); } 
  else
   {tft.print(month);
  }  
//Рік   
tft.println(":");
 tft.setCursor(80, 40);//Поставить курсор Х У
   tft.print(20);
   tft.print(year);    
//День неділі
  tft.setCursor(10, 80);//Поставить курсор Х У
  tft.setTextColor(TFT_GREEN);//Цвет шрифта 
  //tft.setTextSize(2);//Размер шрифта "2"
  week = now.dayOfTheWeek(); 
week_day_out(week);     
                                        
delay(500);           
                      
          }
                  
          }        

