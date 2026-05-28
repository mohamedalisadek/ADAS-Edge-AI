#ifndef BUZZER_PRIVATE_H
#define BUZZER_PRIVATE_H

#include "../../MCAL/GPIO/GPIO_interface.h"

/* ── Pin assignment ── */
#define BUZZER_PORT     GPIO_PORTC
#define BUZZER_PIN      GPIO_PIN5   /* RC5 — active-HIGH buzzer */

/*
 * Timer0 tick period ≈ 204.8 µs  (prescaler 4, 20 MHz)
 * Buzzer pattern counts are in Timer0 overflow units.
 *
 * Pattern type IDs
 */
#define BUZZER_PATTERN_OFF          0U   /* silent                               */
#define BUZZER_PATTERN_SINGLE       1U   /* one short beep then stop             */
#define BUZZER_PATTERN_SLOW         2U   /* slow beep: ~500 ms on / ~500 ms off  */
#define BUZZER_PATTERN_FAST         3U   /* fast beep: ~100 ms on / ~100 ms off  */
#define BUZZER_PATTERN_CONTINUOUS   4U   /* always on                            */

/* Tick counts for each half-period (on or off phase)
 * 1 tick = Timer0 overflow ≈ 204.8 µs @ prescaler 4, 20 MHz
 *
 * BUG FIX: BUZZER_TICKS_SHORT_ON was 49 (~10 ms) which is far too short
 * for a real active buzzer to produce audible sound on hardware.
 * The beep was inaudible/a click during real hardware testing.
 * Changed to 977 ticks ≈ 200 ms — matches the 200 ms pulse in BUZZER_TEST. */
#define BUZZER_TICKS_SHORT_ON   977U   /* ~200 ms (single beep pulse)           */
#define BUZZER_TICKS_SHORT_OFF  977U   /* ~200 ms (silence after single beep)   */
#define BUZZER_TICKS_SLOW_ON   2441U   /* ~500 ms on                            */
#define BUZZER_TICKS_SLOW_OFF  2441U   /* ~500 ms off                           */
#define BUZZER_TICKS_FAST_ON    488U   /* ~100 ms on                            */
#define BUZZER_TICKS_FAST_OFF   488U   /* ~100 ms off                           */

#endif /* BUZZER_PRIVATE_H */
