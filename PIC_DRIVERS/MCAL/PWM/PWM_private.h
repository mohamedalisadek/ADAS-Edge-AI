#ifndef PWM_PRIVATE_H
#define PWM_PRIVATE_H

#include "../../SERVICES/STD_TYPES.h"

/* PIC16F877A PWM Registers */
#define CCP1CON     (*(volatile u8*)0x17)
#define CCP2CON     (*(volatile u8*)0x1D)
#define CCPR1L      (*(volatile u8*)0x15)
#define CCPR1H      (*(volatile u8*)0x16)
#define CCPR2L      (*(volatile u8*)0x1B)
#define CCPR2H      (*(volatile u8*)0x1C)
#define PR2         (*(volatile u8*)0x92)
#define T2CON       (*(volatile u8*)0x12)

/* CCP Modes */
#define PWM_MODE    0x0C

/* T2CON Bits */
#define T2CKPS0     0
#define T2CKPS1     1
#define TMR2ON      2

/* PWM Prescaler Values */
#define PRESCALER_1  0
#define PRESCALER_4  1
#define PRESCALER_16 2

#endif
