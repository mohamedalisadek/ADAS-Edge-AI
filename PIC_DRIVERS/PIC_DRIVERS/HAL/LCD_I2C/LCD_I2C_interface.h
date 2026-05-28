#ifndef LCD_I2C_INTERFACE_H
#define LCD_I2C_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/*
 * 16×2 HD44780 LCD connected via PCF8574 I2C backpack.
 * Uses the I2C MCAL driver (100 kHz).
 *
 * Row 0 = top line, Row 1 = bottom line.
 * Strings longer than 16 chars are silently truncated.
 */

void LCD_I2C_Init(void);
void LCD_I2C_Clear(void);
void LCD_I2C_SetCursor(u8 Row, u8 Col);
void LCD_I2C_PrintChar(u8 Ch);
void LCD_I2C_Print(const u8 *Str);
void LCD_I2C_PrintNumber(s16 Num);              /* prints signed integer          */
void LCD_I2C_PrintRow(u8 Row, const u8 *Str);  /* clear row then print           */
void LCD_I2C_SetBacklight(u8 state);           /* 1 = ON, 0 = OFF (PCF8574 BL)  */

#endif /* LCD_I2C_INTERFACE_H */
