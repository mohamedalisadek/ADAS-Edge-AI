#include "../../SERVICES/BIT_MATH.h"
#include "../../SERVICES/STD_TYPES.h"

/* MCAL drivers */
#include "../USART/USART_Interface.h"
#include "../USART/USART_Private.h"
#include "../EXT_INT/EXT_INT_Interface.h"
#include "../EXT_INT/EXT_INT_Private.h"
#include "../TIMER_0/TIMER_0_Interface.h"
#include "../TIMER_0/TIMER_0_Private.h"
#include "../Registers_Common.h"

/* HAL drivers that have tick/ISR functions called from here */
#include "../../HAL/BUZZER/BUZZER_interface.h"
#include "../../HAL/DOOR_SENSOR/DOOR_SENSOR_interface.h"

/* ── INTCON bit positions not already in Private headers ── */
#define RBIF_BIT    0   /* PORTB change interrupt flag */
#define RBIE_BIT    3   /* PORTB change interrupt enable */

/* ── single ISR — PIC16F877A has one interrupt vector ──── */
void __interrupt() isr(void)
{
    /* ── Timer0 overflow (~204.8 µs) ─────────────────────
     * Drives: Buzzer pattern timing                       */
    if (GET_BIT(INTCON, T0IF_BIT) && GET_BIT(INTCON, T0IE_BIT))
    {
        TIMER_0_ISR();      /* clears T0IF internally      */
        BUZZER_Tick();      /* buzzer pattern step         */
    }

    /* ── External interrupt RB0 (Emergency Stop) ───────── */
    if (GET_BIT(INTCON, INTF_BIT) && GET_BIT(INTCON, INTE_BIT))
    {
        EXT_INT_ISR();      /* clears INTF, calls callback */
    }

    /* ── UART RX ────────────────────────────────────────── */
    /* BUG FIX: RCIF is a hardware status flag — it is set by the USART
     * peripheral whenever a byte arrives, regardless of whether the RX
     * interrupt is enabled (RCIE).  Without the RCIE guard, UART_ISR()
     * would be called here even when RX interrupts are disabled, causing
     * phantom callbacks.  All other sources in this ISR correctly check
     * both the flag AND the enable bit (T0IF/T0IE, INTF/INTE, RBIF/RBIE).
     * PIE1 (0x8C) and RCIE are already defined via USART_Private.h above. */
    if (GET_BIT(PIR1, RCIF) && GET_BIT(PIE1, RCIE))
    {
        UART_ISR();         /* reads RCREG, calls callback */
    }

    /* ── PORTB change interrupt (RB4–RB5) ───────────────
     * Fires when any of: Door1(RB4), Door2(RB5) changes. */
    if (GET_BIT(INTCON, RBIF_BIT) && GET_BIT(INTCON, RBIE_BIT))
    {
        /* Reading PORTB inside the ISR latches the change and
         * is required by the PIC16F877A to clear the mismatch
         * that triggers RBIF. */
        DOOR_SENSOR_ISR();

        CLR_BIT(INTCON, RBIF_BIT);   /* clear flag after reading */
    }
}
