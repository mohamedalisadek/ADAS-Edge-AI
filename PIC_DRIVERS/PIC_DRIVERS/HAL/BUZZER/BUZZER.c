#include "BUZZER_interface.h"
#include "BUZZER_private.h"
#include "../../MCAL/GPIO/GPIO_interface.h"
#include "../../MCAL/Registers_Common.h"
#include "../../SERVICES/BIT_MATH.h"

/* ── state (written by app, read by BUZZER_Tick in ISR) ── */
static volatile u8  buzzer_pattern  = BUZZER_PATTERN_OFF;
static volatile u16 buzzer_tick_cnt = 0;
static volatile u8  buzzer_phase    = 0;   /* 0 = ON phase, 1 = OFF phase */
static volatile u8  buzzer_done     = 0;   /* for single-beep auto-stop   */

/*
 * BUZZER_ENTER_CRITICAL / BUZZER_EXIT_CRITICAL
 *
 * BUG FIX: Race condition — the setter functions write to buzzer_pattern,
 * buzzer_tick_cnt, and buzzer_phase in separate store instructions.  If the
 * Timer0 ISR fires between any two of those writes it will observe an
 * inconsistent mix of old and new state, producing a wrong timing glitch.
 *
 * Fix: briefly disable global interrupts while updating the shared state.
 * On PIC16 this is a single BCF INTCON,GIE instruction, so the window is
 * at most one Timer0 overflow (204.8 µs) — negligible for audio patterns.
 *
 * NOTE: These macros save/restore GIE rather than unconditionally enabling
 * it on exit, so they are safe even if called from within an ISR callback.
 */
#define BUZZER_ENTER_CRITICAL()  do { CLR_BIT(INTCON, GIE_BIT); } while(0)
#define BUZZER_EXIT_CRITICAL()   do { SET_BIT(INTCON, GIE_BIT); } while(0)

/* ── init ───────────────────────────────────────────────── */

void BUZZER_Init(void)
{
    GPIO_SetPinDirection(BUZZER_PORT, BUZZER_PIN, GPIO_OUTPUT);
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_LOW);
}

/* ── pattern setters ────────────────────────────────────── */

void BUZZER_Off(void)
{
    BUZZER_ENTER_CRITICAL();
    buzzer_pattern  = BUZZER_PATTERN_OFF;
    buzzer_tick_cnt = 0;
    buzzer_phase    = 0;
    buzzer_done     = 0;
    BUZZER_EXIT_CRITICAL();
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_LOW);
}

void BUZZER_SingleBeep(void)
{
    BUZZER_ENTER_CRITICAL();
    buzzer_pattern  = BUZZER_PATTERN_SINGLE;
    buzzer_tick_cnt = 0;
    buzzer_phase    = 0;
    buzzer_done     = 0;
    BUZZER_EXIT_CRITICAL();
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
}

void BUZZER_SlowBeep(void)
{
    BUZZER_ENTER_CRITICAL();
    buzzer_pattern  = BUZZER_PATTERN_SLOW;
    buzzer_tick_cnt = 0;
    buzzer_phase    = 0;
    buzzer_done     = 0;
    BUZZER_EXIT_CRITICAL();
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
}

void BUZZER_FastBeep(void)
{
    BUZZER_ENTER_CRITICAL();
    buzzer_pattern  = BUZZER_PATTERN_FAST;
    buzzer_tick_cnt = 0;
    buzzer_phase    = 0;
    buzzer_done     = 0;
    BUZZER_EXIT_CRITICAL();
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
}

void BUZZER_Continuous(void)
{
    /* BUG FIX: reset all state variables for consistency — previously only
     * buzzer_pattern was updated, leaving stale tick_cnt / phase / done. */
    BUZZER_ENTER_CRITICAL();
    buzzer_pattern  = BUZZER_PATTERN_CONTINUOUS;
    buzzer_tick_cnt = 0;
    buzzer_phase    = 0;
    buzzer_done     = 0;
    BUZZER_EXIT_CRITICAL();
    GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
}

/* ── tick (called from Timer0 ISR every ~204.8 µs) ─────── */

void BUZZER_Tick(void)
{
    u16 on_ticks;
    u16 off_ticks;

    switch (buzzer_pattern)
    {
        case BUZZER_PATTERN_OFF:
        case BUZZER_PATTERN_CONTINUOUS:
            return;   /* nothing to do, pin already set correctly */

        case BUZZER_PATTERN_SINGLE:
            if (buzzer_done) return;
            on_ticks  = BUZZER_TICKS_SHORT_ON;
            off_ticks = BUZZER_TICKS_SHORT_OFF;
            break;

        case BUZZER_PATTERN_SLOW:
            on_ticks  = BUZZER_TICKS_SLOW_ON;
            off_ticks = BUZZER_TICKS_SLOW_OFF;
            break;

        case BUZZER_PATTERN_FAST:
            on_ticks  = BUZZER_TICKS_FAST_ON;
            off_ticks = BUZZER_TICKS_FAST_OFF;
            break;

        default:
            return;
    }

    buzzer_tick_cnt++;

    if (buzzer_phase == 0)   /* ON phase */
    {
        if (buzzer_tick_cnt >= on_ticks)
        {
            GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_LOW);
            buzzer_phase    = 1;
            buzzer_tick_cnt = 0;
        }
    }
    else                     /* OFF phase */
    {
        if (buzzer_tick_cnt >= off_ticks)
        {
            if (buzzer_pattern == BUZZER_PATTERN_SINGLE)
            {
                buzzer_done = 1;    /* single beep complete — stay off */
                return;
            }
            GPIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
            buzzer_phase    = 0;
            buzzer_tick_cnt = 0;
        }
    }
}
