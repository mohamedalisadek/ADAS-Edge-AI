#ifndef TIMER_0_PRIVATE_H
#define TIMER_0_PRIVATE_H

#include "../Registers_Common.h"

#define TMR0        (*((volatile u8*)0x01))

// INTCON bit definitions
#define T0IE_BIT    5
#define T0IF_BIT    2

// OPTION_REG bit definitions
#define PS0_BIT     0
#define PS1_BIT     1
#define PS2_BIT     2
#define PSA_BIT     3
#define T0CS_BIT    5   /* BUG FIX: was 4 — T0CS is bit 5 of OPTION_REG (PIC16F877A datasheet Table 2-1) */
#define T0SE_BIT    4   /* BUG FIX: was 5 — T0SE is bit 4 of OPTION_REG */

#endif
