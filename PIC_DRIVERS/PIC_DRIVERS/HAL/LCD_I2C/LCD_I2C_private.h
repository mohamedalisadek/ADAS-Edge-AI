#ifndef LCD_I2C_PRIVATE_H
#define LCD_I2C_PRIVATE_H

/*
 * PCF8574 I2C backpack wiring to HD44780 LCD (standard Arduino mapping):
 *
 *  PCF8574 pin | I2C byte bit | LCD signal
 *  ────────────┼──────────────┼───────────
 *   P0         |  bit 0       |  RS  (Register Select)
 *   P1         |  bit 1       |  RW  (always 0 — write-only)
 *   P2         |  bit 2       |  E   (Enable strobe)
 *   P3         |  bit 3       |  BL  (Backlight; 1=on)
 *   P4         |  bit 4       |  D4
 *   P5         |  bit 5       |  D5
 *   P6         |  bit 6       |  D6
 *   P7         |  bit 7       |  D7
 */

/* Control bits */
#define LCD_RS_BIT   0   /* Register Select: 0=command, 1=data */
#define LCD_RW_BIT   1   /* Read/Write: always driven LOW       */
#define LCD_EN_BIT   2   /* Enable strobe                       */
#define LCD_BL_BIT   3   /* Backlight                           */

/* Data nibble — explicit bit names (merged from Drivers_last_updated/HAL/LCD) */
#define LCD_D4_BIT   4
#define LCD_D5_BIT   5
#define LCD_D6_BIT   6
#define LCD_D7_BIT   7

/* Default I2C address of PCF8574 (A0=A1=A2=GND) */
#define LCD_I2C_ADDRESS  0x27U

/* LCD geometry */
#define LCD_ROWS    2U
#define LCD_COLS   16U

/* HD44780 command codes */
#define LCD_CMD_CLEAR        0x01U
#define LCD_CMD_HOME         0x02U
#define LCD_CMD_ENTRY_MODE   0x06U   /* increment cursor, no shift        */
#define LCD_CMD_DISPLAY_OFF  0x08U   /* display off (needed before clear) */
#define LCD_CMD_DISPLAY_ON   0x0CU   /* display on, cursor off, no blink  */
#define LCD_CMD_4BIT_2LINE   0x28U   /* 4-bit, 2 lines, 5×8 font          */
#define LCD_CMD_SET_DDRAM    0x80U   /* OR with DDRAM address             */

/* Row-start addresses for DDRAM */
#define LCD_ROW0_ADDR        0x00U
#define LCD_ROW1_ADDR        0x40U

/* Backlight default (on) */
#define LCD_BL_ON   (1U << LCD_BL_BIT)

#endif /* LCD_I2C_PRIVATE_H */
