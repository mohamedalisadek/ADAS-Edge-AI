#ifndef DELAY_H
#define DELAY_H

/**
 * @file  DELAY.h
 * @brief Delay utilities — PIC16F877A @ 20 MHz
 *
 * Uses XC8's built-in __delay_us() / __delay_ms() macros, which are
 * cycle-accurate regardless of compiler optimisation level, unlike the
 * previous hand-counted software loops whose iteration time varied with
 * XC8 -O0 vs -O2 settings and caused inaccurate sensor results.
 *
 * Best practice ported from Drivers_last_updated/SERVICES/config.h.
 *
 * MCU_CONFIG.h must be included first — it defines _XTAL_FREQ which
 * XC8 needs to compute the loop iteration counts for __delay_ms /
 * __delay_us at compile time.
 *
 * Legacy names DELAY_ms() and DELAY_us10() are kept as thin macros so
 * all existing call sites compile without modification.
 */

#include "MCU_CONFIG.h"   /* defines _XTAL_FREQ = 20000000UL — must precede xc.h */
#include <xc.h>
#include "STD_TYPES.h"

/* Cycle-accurate millisecond delay */
#define DELAY_ms(ms)    __delay_ms(ms)

/* Cycle-accurate 10 µs delay — HC-SR04 trigger pulse duration */
#define DELAY_us10()    __delay_us(10)

#endif /* DELAY_H */
