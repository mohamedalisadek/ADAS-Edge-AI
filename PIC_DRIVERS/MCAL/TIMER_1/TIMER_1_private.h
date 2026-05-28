#ifndef TIMER_1_PRIVATE_H
#define TIMER_1_PRIVATE_H

/*
 * TIMER_1_private.h — PIC16F877A Timer1 bit-position constants
 *
 * ⚠ BUG FIX (from Drivers_last_updated best practice):
 *   This file previously defined TMR1L, TMR1H, T1CON as raw pointer
 *   macros:  #define TMR1L  (*(volatile u8*)0x0E)
 *
 *   Those macros conflict with xc.h SFR declarations. The preprocessor
 *   expands the macro inside xc.h's  "extern volatile unsigned char TMR1L;"
 *   declaration, producing garbled C that silently breaks Timer1 access
 *   when <xc.h> is included in the same translation unit (e.g. ULTRASONIC.c
 *   needed xc.h for ADCON1, PIR1bits — the conflict caused TMR1 reads/writes
 *   to malfunction, explaining null distance readings).
 *
 *   ALL register address macros removed.
 *   Any .c file that touches Timer1 SFRs must #include <xc.h>, which
 *   provides TMR1L, TMR1H, T1CON, PIR1, etc. with proper bank-aware
 *   access for the PIC16 architecture.
 */

/* T1CON bit positions — used with BIT_MATH SET_BIT / CLR_BIT
 * ⚠ Names use _BIT suffix to avoid clashing with xc.h, which declares
 *   TMR1ON, T1CKPS0, T1CKPS1, TMR1CS, T1SYNC, T1OSCEN as __bit variables
 *   for the PIC16F877A.  Using the bare names here caused 18 macro-expansion
 *   errors inside pic16f877a.h's T1CONbits struct definitions.              */
#define TMR1ON_BIT      0   /* Timer1 enable                           */
#define TMR1CS_BIT      1   /* Clock source: 0 = Fosc/4, 1 = external */
#define T1SYNC_BIT      2   /* External clock sync (irrelevant internal)*/
#define T1OSCEN_BIT     3   /* Timer1 oscillator enable                */
#define T1CKPS0_BIT     4   /* Prescaler select bit 0                  */
#define T1CKPS1_BIT     5   /* Prescaler select bit 1                  */

/* Prescaler values for T1CKPS1:T1CKPS0 field */
#define TIMER1_PRESCALER_1   0x00   /* 1:1 — tick = 0.2 µs @ 20 MHz */
#define TIMER1_PRESCALER_2   0x01   /* 1:2 — tick = 0.4 µs          */
#define TIMER1_PRESCALER_4   0x02   /* 1:4 — tick = 0.8 µs          */
#define TIMER1_PRESCALER_8   0x03   /* 1:8 — tick = 1.6 µs          */

#endif /* TIMER_1_PRIVATE_H */
