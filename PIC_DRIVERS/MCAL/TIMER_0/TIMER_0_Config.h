#ifndef TIMER_0_CONFIG_H
#define TIMER_0_CONFIG_H

#include "../../SERVICES/STD_TYPES.h"

/* Timer0 Mode Configuration */
#define TIMER_0_MODE TIMER_0_MODE_TIMER

/* Timer0 Prescaler Configuration
 * Prescaler 4  →  1 overflow = 256 × 4 × (1/5MHz) ≈ 204.8 µs
 * Used by: Servo PWM (50 Hz, 98 overflows/period) and Buzzer tick */
#define TIMER_0_PRESCALER TIMER_0_PRESCALER_4

/* Mode Options */
#define TIMER_0_MODE_TIMER 0
#define TIMER_0_MODE_COUNTER 1

/* Prescaler Options */
#define TIMER_0_PRESCALER_2 0
#define TIMER_0_PRESCALER_4 1
#define TIMER_0_PRESCALER_8 2
#define TIMER_0_PRESCALER_16 3
#define TIMER_0_PRESCALER_32 4
#define TIMER_0_PRESCALER_64 5
#define TIMER_0_PRESCALER_128 6
#define TIMER_0_PRESCALER_256 7

#endif
