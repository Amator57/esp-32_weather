//#pragma once
#include "globals.h"
#include "PUSH.h"
/*
#ifndef UVOD
#define UVOD      32
#endif

#ifndef MINUS
#define MINUS      33
#endif

#ifndef PLUS
#define PLUS       25
#endif
*/
#define BUTTON_PROTECTION 50
#define TIME_STEP 10
#define BUTTON_LONG_PRESS_TIME 500

unsigned char plus (void)
{
char i;
 //int sensorVal = digitalRead(PLUS);
if (!digitalRead(PLUS))
        {
delay(100);

if (!digitalRead(PLUS))
                {
i=1;
                }     
       
 while (!digitalRead(PLUS)); 
        }            
  else i=0;
         
 return i;       
} 
// =========================
  unsigned char minus (void)
{
char i;
if (!digitalRead(MINUS))
        {
delay(100);
if (!digitalRead(MINUS))
                {
i=1;
                }     
        
while (!digitalRead(MINUS));
        }     
   else i=0; 
                 
 return i;       
} 
// =========================
  unsigned char uvod (void)
{
char i;
if (!digitalRead(UVOD))
        {
delay(100);
if (!digitalRead(UVOD))
                {
i=1;
                }
while (!digitalRead(UVOD));                     
        }
             
  else i=0; 
      
 return i;       
}
 // =========================
 unsigned char Entrance (void)
{
	char x;
	char y;
	char z;
x=	plus();
y= minus();

if (x && y)
                {
z=1;
                }     
        
  else z=0;       
 
 return z;       			
}


unsigned char button (char x)
 {
 unsigned char n=0;
  //проверка нажатия кнопки
  if(!digitalRead(x)){
    //пауза для защиты от дребезга
    delay(BUTTON_PROTECTION);  
    //повторный опрос кнопки
    if(!digitalRead(x)){
     n =1;
      //ничего не делаем, пока кнопка нажата
      while(digitalRead(x) == LOW);
 
    }
  }
  return n;
}
unsigned char long_button (char x){
  //для измерения времени нажатия 
  uint16_t buttonPressTime = 0;

  //проверка нажатия кнопки
  while(digitalRead(x) == LOW){
    //шаг по шкале времени
    delay(TIME_STEP);  
    //считаем время
    buttonPressTime += TIME_STEP;
    //это нужно, чтоб счетчик не переполнился, если кто-то уснет на кнопке
    if(buttonPressTime > BUTTON_LONG_PRESS_TIME)
      //buttonPressTime = BUTTON_LONG_PRESS_TIME;
   buttonPressTime = 1;
  }

 return buttonPressTime;
}
/*
  //проверяем длинное нажатие кнопки
  if(buttonPressTime >= BUTTON_LONG_PRESS_TIME)
    return buttonLongPress;



  //проверяем короткое нажатие кнопки
  if(buttonPressTime >= BUTTON_SHORT_PRESS_TIME)
    return buttonShortPress;

  //сообщаем, что кнопку не нажимали
  return buttonNotPress;
  */
/* 
//Запропоновано GPT
  unsigned char button (char x) {
  unsigned char n = 0;
  if (!digitalRead(x)) {
    delay(BUTTON_PROTECTION);
    if (!digitalRead(x)) {
      n = 1;
      unsigned long t0 = millis();
      // Чекаємо, поки кнопку відпустять або мине таймаут
      while (digitalRead(x) == LOW) {
        if (millis() - t0 > 1000) break; // 1 секунда таймаут
      }
    }
  }
  return n;
}
*/