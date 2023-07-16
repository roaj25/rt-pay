#ifndef LCD_H
#define LCD_H
/*
+++   Nima Askari
+++   www.github.com/NimaLTD
+++   www.instagram.com/github.NimaLTD
+++   Version: 1.1.0
*/


#include <rtdef.h>

#define _LCD_I2C_BUS    "i2c1"
#define _LCD_I2C_ADD    0x3F

#define _LCD_D7_PIN 7
#define _LCD_D6_PIN 6
#define _LCD_D5_PIN 5
#define _LCD_D4_PIN 4
#define _LCD_BT_PIN 3
#define _LCD_E_PIN  2
#define _LCD_RW_PIN 1
#define _LCD_RS_PIN 0

#define _LCD_COLS   16
#define _LCD_ROWS   2

void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(void);
void LCD_Puts(rt_uint8_t x, rt_uint8_t y, char* str);
void LCD_BlinkOn(void);
void LCD_BlinkOff(void);
void LCD_CursorOn(void);
void LCD_CursorOff(void);
void LCD_ScrollLeft(void);
void LCD_ScrollRight(void);
void LCD_CreateChar(rt_uint8_t location, rt_uint8_t* data);
void LCD_PutCustom(rt_uint8_t x, uint8_t y, rt_uint8_t location);
void LCD_Put(rt_uint8_t Data);


#endif
