#ifndef TIMER_1_INTERFACE_H
#define TIMER_1_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/*
 * Timer1 — used for ultrasonic echo pulse width measurement.
 *
 * Config: internal clock (Fosc/4), prescaler 1:1
 *   → 1 count = 0.2 µs  (at 20 MHz crystal)
 *   → max range before overflow: 65535 × 0.2 µs = 13.1 ms  (≈ 226 cm)
 *
 * Usage pattern in ULTRASONIC driver:
 *   TIMER1_Clear();
 *   TIMER1_Start();
 *   ... wait for echo to go LOW ...
 *   TIMER1_Stop();
 *   count = TIMER1_GetCount();
 *   distance_cm = count / 290;   (290 = 58 µs/cm ÷ 0.2 µs/count)
 */

void TIMER1_Init(void);
void TIMER1_Start(void);
void TIMER1_Stop(void);
void TIMER1_Clear(void);
u16  TIMER1_GetCount(void);

/* Enhanced API (adopted from cross-team review):
 *   SetValue    — preload TMR1H:TMR1L before starting (stop timer first)
 *   SetPrescaler— pass 1, 2, 4, or 8 as a plain integer              */
void TIMER1_SetValue(u16 value);
void TIMER1_SetPrescaler(u8 prescaler);

#endif /* TIMER_1_INTERFACE_H */
