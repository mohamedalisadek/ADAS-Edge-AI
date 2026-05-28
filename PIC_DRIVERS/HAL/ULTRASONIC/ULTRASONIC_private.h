#ifndef ULTRASONIC_PRIVATE_H
#define ULTRASONIC_PRIVATE_H

/*
 * HC-SR04 single-sensor configuration — PIC16F877A @ 20 MHz
 *
 * Ported from Drivers_last_updated/HAL/HC-SR04/SR04.h best practices:
 *   - Timer1 prescaler 1:8  (tick = 1.6 µs instead of 0.2 µs)
 *   - Overflow-flag timeout instead of inaccurate software counters
 *   - Direct xc.h register access; no conflicting private macros
 *
 * TRIG = RA1  (digital output)
 * ECHO = RA2  (digital input  — requires ADCON1 = 0x07)
 */

/* ─── Timer1 conversion factor ──────────────────────────────────────────
 * Prescaler 1:8, Fosc = 20 MHz → tick = 8 / (20 MHz / 4) = 1.6 µs/tick
 * Sound round-trip per cm = 58 µs → ticks/cm = 58 / 1.6 = 36.25 → 36
 */
#define ULTRASONIC_COUNT_PER_CM   36U

/* Maximum valid distance (cm).  Beyond this the reading is discarded. */
#define ULTRASONIC_MAX_DIST_CM    200U

/* Timeout waiting for ECHO to go HIGH: 500 iterations × __delay_us(10) = 5 ms */
#define ULTRASONIC_ECHO_TIMEOUT   500U

/*
 * Sentinels returned on measurement failure.
 *
 * ULTRASONIC_OUT_OF_RANGE (400):
 *   Pass 2 timed out — ECHO went HIGH but never returned LOW.
 *   Means: sensor responded to trigger but object is beyond
 *   ~3.4 m (Timer1 overflow at 104 ms), OR ECHO pin stuck HIGH.
 *
 * ULTRASONIC_NO_SENSOR (401):
 *   Pass 1 timed out — ECHO never went HIGH after the trigger pulse.
 *   Means: sensor is not connected, not powered, or TRIG wiring is open.
 *   This is the "no hardware" sentinel and should show "NO SENSOR" on LCD.
 *
 * Both values are larger than any valid distance or FCW threshold so a
 * broken sensor is NEVER silently treated as a clear road.
 *
 * ⚠  Old code returned 0 on timeout — FCW treated 0 as "clear road".
 */
#define ULTRASONIC_OUT_OF_RANGE   400U   /* Pass 2 timeout — echo stuck HIGH  */
#define ULTRASONIC_NO_SENSOR      401U   /* Pass 1 timeout — no echo at all   */

#endif /* ULTRASONIC_PRIVATE_H */
