#ifndef BUZZER_INTERFACE_H
#define BUZZER_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/*
 * Active Buzzer driver — pin RC5, active HIGH.
 *
 * Patterns are driven from the Timer0 ISR via BUZZER_Tick().
 * Call BUZZER_Tick() once every Timer0 overflow (~204.8 µs).
 *
 * Pattern IDs (use the BUZZER_PATTERN_* constants from BUZZER_private.h
 * or the wrapper functions below — prefer the wrapper functions).
 */

void BUZZER_Init(void);

/* Pattern setters — call any time; take effect immediately */
void BUZZER_Off(void);
void BUZZER_SingleBeep(void);      /* one short beep, then auto-off     */
void BUZZER_SlowBeep(void);        /* repeated slow beep (pre-trip warn) */
void BUZZER_FastBeep(void);        /* repeated fast beep (collision warn) */
void BUZZER_Continuous(void);      /* always on                          */

/* Called from Timer0 ISR — do NOT call from application code */
void BUZZER_Tick(void);

#endif /* BUZZER_INTERFACE_H */
