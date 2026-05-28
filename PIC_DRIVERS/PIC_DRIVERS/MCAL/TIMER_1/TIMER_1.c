/**
 * @file  TIMER_1.c
 * @brief Timer1 abstraction layer — PIC16F877A
 *
 * Uses xc.h for all SFR access (TMR1L, TMR1H, T1CON).
 * TIMER_1_private.h now contains ONLY bit-position constants,
 * NOT raw register macros — those macros conflicted with xc.h
 * when both were included in the same translation unit.
 */

#include "TIMER_1_interface.h"
#include "TIMER_1_private.h"
#include "../../SERVICES/BIT_MATH.h"
#include <xc.h>     /* provides TMR1L, TMR1H, T1CON, PIR1, etc. */

void TIMER1_Init(void)
{
    /* Internal clock (Fosc/4), prescaler 1:1, Timer1 OFF */
    T1CON = 0x00;
    TMR1H = 0x00;
    TMR1L = 0x00;
    PIR1bits.TMR1IF = 0;
}

void TIMER1_Start(void)
{
    PIR1bits.TMR1IF = 0;       /* clear stale overflow flag first */
    SET_BIT(T1CON, TMR1ON_BIT);
}

void TIMER1_Stop(void)
{
    CLR_BIT(T1CON, TMR1ON_BIT);
}

void TIMER1_Clear(void)
{
    /* Stop timer before clearing to prevent a carry mid-write */
    CLR_BIT(T1CON, TMR1ON_BIT);
    TMR1H = 0x00;
    TMR1L = 0x00;
    PIR1bits.TMR1IF = 0;
}

u16 TIMER1_GetCount(void)
{
    /* PIC16F877A datasheet §6.0:
     * Reading TMR1L first latches TMR1H into a shadow register so
     * both bytes are consistent even while the counter is running.
     * Never read TMR1H first — that causes phantom high-byte reads. */
    u8 low  = TMR1L;   /* latches TMR1H shadow */
    u8 high = TMR1H;   /* reads shadow — always consistent          */
    return (u16)((u16)high << 8) | (u16)low;
}

void TIMER1_SetValue(u16 value)
{
    /* Write high byte first — safer on a live counter (no shadow
     * register for writes, so stop timer before calling this).    */
    TMR1H = (u8)(value >> 8U);
    TMR1L = (u8)(value & 0xFFU);
}

void TIMER1_SetPrescaler(u8 prescaler)
{
    u8 bits;

    switch (prescaler)
    {
        case 1:  bits = TIMER1_PRESCALER_1; break;
        case 2:  bits = TIMER1_PRESCALER_2; break;
        case 4:  bits = TIMER1_PRESCALER_4; break;
        case 8:  bits = TIMER1_PRESCALER_8; break;
        default: bits = TIMER1_PRESCALER_1; break;
    }

    /* Clear T1CKPS[5:4] then write new value; preserve all other bits */
    T1CON = (T1CON & 0xCFU) | (u8)(bits << T1CKPS0_BIT);
}
