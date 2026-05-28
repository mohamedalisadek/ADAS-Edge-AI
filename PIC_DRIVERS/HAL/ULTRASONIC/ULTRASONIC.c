/**
 * @file   ULTRASONIC.c
 * @brief  HC-SR04 single-sensor driver — PIC16F877A @ 20 MHz
 *
 * Rewritten after comparing with Drivers_last_updated/HAL/HC-SR04/SR04.c
 * and ES_Course_Labs/HAL/HC-SR04/SR04.c (2026-05-14).
 *
 * Key changes from previous version:
 *
 *   1. Two-pass Timer1 measurement (ES_Course_Labs pattern)
 *      Pass 1: Timer1 used to TIMEOUT the echo-HIGH wait.
 *              Previous driver used __delay_us(10) x 500 = 5 ms.
 *              If the sensor takes longer (slow clone, noisy line),
 *              the 5 ms loop exits early and returns OUT_OF_RANGE
 *              before Timer1 even starts.  Timer1 at 1:8 prescaler
 *              overflows after ~104 ms — a much safer timeout.
 *      Pass 2: Timer1 reset and restarted to measure echo-LOW wait
 *              (the actual round-trip time).
 *
 *   2. T1CON written as one complete value (Drivers_last_updated pattern)
 *      Writing T1CON = 0x31 in a single instruction (prescaler 1:8 +
 *      TMR1ON=1) instead of relying on prescaler bits preserved from
 *      ULTRASONIC_Init() eliminates any possible stale-state issue.
 *
 *   3. No Timer1 interrupt required
 *      ES_Course_Labs uses TMR1IE + callback; this driver polls TMR1IF
 *      directly so no ISR handler change is needed.
 *
 *   4. TIMER_1 abstraction layer no longer used in GetDistance
 *      Direct SFR access matches both alternate drivers and avoids
 *      any abstraction-layer overhead or state assumptions.
 *
 * PIN ASSIGNMENT:
 *   TRIG = RA1  (digital output)
 *   ECHO = RA2  (digital input — ADCON1 = 0x07 required for digital read)
 *
 * Timer1 @ prescaler 1:8, 20 MHz:
 *   tick = 1.6 µs,  36 ticks/cm,  overflow after ~104 ms
 */

#include "ULTRASONIC_interface.h"
#include "ULTRASONIC_private.h"
#include "../../SERVICES/MCU_CONFIG.h"
#include <xc.h>

/* T1CON value: internal clock (TMR1CS=0), prescaler 1:8 (T1CKPS=11),
 * timer ON (TMR1ON=1).  Written fresh on every measurement so the
 * prescaler is never assumed to be correct from a previous call.
 *   bit 0 (TMR1ON)   = 1
 *   bit 1 (TMR1CS)   = 0  internal Fosc/4
 *   bit 2 (T1SYNC)   = 0
 *   bit 3 (T1OSCEN)  = 0
 *   bit 5:4 (T1CKPS) = 11 → prescaler 1:8
 *   = 0b00110001 = 0x31                                               */
#define T1CON_PS8_ON   0x31U

/* ── helper: reset and start Timer1 (prescaler 1:8) ─────────────────
 * Stops first, clears count and flag, then writes T1CON as one value. */
#define TMR1_RESTART() do {           \
    T1CONbits.TMR1ON = 0;            \
    TMR1H = 0U;                      \
    TMR1L = 0U;                      \
    PIR1bits.TMR1IF  = 0;            \
    T1CON = T1CON_PS8_ON;            \
} while (0)

/* ── helper: stop Timer1 and read 16-bit count ──────────────────────
 * Read TMR1L first — latches TMR1H shadow on PIC16F877A (§6.0).     */
#define TMR1_STOP_READ(var) do {                              \
    T1CONbits.TMR1ON = 0;                                    \
    (var) = (u16)(((u16)TMR1H << 8U) | (u16)TMR1L);         \
} while (0)

/* ------------------------------------------------------------------ */
void ULTRASONIC_Init(void)
{
    /* ADCON1 = 0x07 → PCFG = 0111:
     *   AN0 (RA0) = analog  (LDR on ADC CH0 — must stay analog)
     *   AN1 (RA1) = digital (TRIG output — not affected either way)
     *   AN2 (RA2) = digital (ECHO input  — MUST be digital)         */
    ADCON1 = 0x07;

    /* TRIG = RA1 output, idle LOW */
    TRISAbits.TRISA1 = 0;
    PORTAbits.RA1    = 0;

    /* ECHO = RA2 input */
    TRISAbits.TRISA2 = 1;

    /* Timer1: stop, clear, no interrupt — GetDistance sets T1CON fresh */
    T1CONbits.TMR1ON = 0;
    TMR1H = 0U;
    TMR1L = 0U;
    PIR1bits.TMR1IF  = 0;
    PIE1bits.TMR1IE  = 0;   /* interrupt NOT used — we poll TMR1IF    */
}

/* ------------------------------------------------------------------ */
/**
 * @brief  Fire the sensor and return measured distance in cm.
 *
 * Two-pass Timer1 measurement (adapted from ES_Course_Labs SR04.c):
 *
 *   Pass 1 — wait for ECHO HIGH:
 *     Timer1 runs while waiting for RA2 to go HIGH.
 *     If Timer1 overflows (~104 ms) before ECHO goes HIGH → no sensor.
 *
 *   Pass 2 — measure ECHO HIGH duration:
 *     Timer1 reset and restarted.  Counts until RA2 falls LOW or
 *     Timer1 overflows (~104 ms, object beyond measurable range).
 *
 * @return 1–200 cm (valid), or ULTRASONIC_OUT_OF_RANGE (400) on fault.
 */
u16 ULTRASONIC_GetDistance(void)
{
    u16 ticks;

    /* ── 1. Send 10 µs trigger pulse ────────────────────────────── */
    PORTAbits.RA1 = 1;
    __delay_us(10);
    PORTAbits.RA1 = 0;

    /* ── 2. Wait for ECHO to go HIGH — Timer1 as timeout ────────── *
     * HC-SR04: ECHO rises ~150–500 µs after TRIG falls.            *
     * Previous driver used __delay_us(10) x 500 = 5 ms — too short *
     * for slow clones or noisy lines.  Timer1 at 1:8 = ~104 ms.   */
    TMR1_RESTART();
    while (!PORTAbits.RA2 && !PIR1bits.TMR1IF)
        ;   /* spin — Timer1 overflow OR echo HIGH exits this loop   */
    T1CONbits.TMR1ON = 0;

    if (PIR1bits.TMR1IF)            /* Timer1 overflowed: no echo   */
    {
        PIR1bits.TMR1IF = 0;
        __delay_ms(40);
        return ULTRASONIC_NO_SENSOR;   /* Pass 1 fail: RA2 never went HIGH */
    }

    /* ── 3. Measure ECHO HIGH duration ──────────────────────────── */
    TMR1_RESTART();
    while (PORTAbits.RA2 && !PIR1bits.TMR1IF)
        ;   /* spin — Timer1 overflow OR echo LOW exits this loop    */
    TMR1_STOP_READ(ticks);

    if (PIR1bits.TMR1IF)            /* overflow: object out of range */
    {
        PIR1bits.TMR1IF = 0;
        __delay_ms(40);
        return ULTRASONIC_OUT_OF_RANGE;
    }
    PIR1bits.TMR1IF = 0;

    /* ── 4. Inter-reading cooldown (prevents echo cross-talk) ───── */
    __delay_ms(40);

    /* ── 5. Convert ticks → cm ──────────────────────────────────── *
     * 36 ticks/cm (1.6 µs/tick × 36 = 57.6 µs ≈ 58 µs round-trip) */
    u16 dist = ticks / ULTRASONIC_COUNT_PER_CM;
    return (dist > ULTRASONIC_MAX_DIST_CM) ? ULTRASONIC_OUT_OF_RANGE : dist;
}
