#include "globals.h" // якщо тут є tft, rtc тощо
#include "Set_month.h"

void set_real_month (void)//                       
 {
  int8_t  month;
    unsigned char t=1;
    unsigned char m=1;
 unsigned char f_enter = 0;
 unsigned char f_plus = 0;
 unsigned char f_minus = 0;
 DateTime now = rtc.now();
 month = now.month();

    while( m )
    {
        f_plus = button(PLUS);
        f_minus = button(MINUS);
        f_enter = f_enter + button(UVOD);
        

          if (!f_enter)
          {                   
                               
          month = month + f_plus;
          month = month - f_minus;
          f_plus =0; 
          f_minus = 0;
                    if (month > 12) month = 1;
                    if (month < 1) month = 12;
          
        
	  tft.fillScreen(TFT_BLACK);//Очистить дисплей“
	  tft.setTextColor(TFT_BLUE);//Колір шрифта
      //tft.setTextSize(2);//Розмір шрифта "2" 	  
  tft.setCursor(10, 40);//Встановити курсор в позицію Х У
if(day<10)
  {
   tft.print(0);
   tft.print(day); } 
  else
   {
    tft.print(day);
   }
         
tft.println(":");    

tft.setCursor(45, 40);//Поставить курсор Х У

  
  if(month<10)
  {
   tft.print(0);
   tft.print(month);
  }
  else
   {
	   tft.print(month);
  } 		
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