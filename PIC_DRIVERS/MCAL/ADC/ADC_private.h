#ifndef ADC_PRIVATE_H
#define ADC_PRIVATE_H

#include "../../SERVICES/STD_TYPES.h"

/* PIC16F877A ADC Registers */
#define ADCON0  (*(volatile u8*)0x1F)
#define ADCON1  (*(volatile u8*)0x9F)
#define ADRESH  (*(volatile u8*)0x1E)
#define ADRESL  (*(volatile u8*)0x9E)

/* ADCON0 Bits */
#define ADON    0
#define GO_DONE 2
#define CHS0    3
#define CHS1    4
#define CHS2    5
#define ADCS0   6
#define ADCS1   7

/* ADCON1 Bits */
#define PCFG0   0
#define PCFG1   1
#define PCFG2   2
#define PCFG3   3
#define ADFM    7

/* ADC Configuration Value for ADCON1
 * PCFG = 1110 (0x0E): AN0 analog only, AN1-AN7 digital.
 * This lets RA1(TRIG) and RA2(ECHO) work as digital GPIO.
 * ADC.c also sets ADFM=1 for right-justified 10-bit result. */
#define ADCON1_CONFIG  0x0E

#endif
