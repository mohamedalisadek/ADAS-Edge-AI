#ifndef MCU_CONFIG_H
#define MCU_CONFIG_H

/* PIC16F877A MCU Configuration */

/* Crystal Frequency */
#define MCU_FOSC    20000000UL  /* 20 MHz crystal (matches the 20 MHz oscillator on the ADAS board) */
#define _XTAL_FREQ  20000000UL  /* Required by XC8 __delay_ms() / __delay_us() macros */

#endif
