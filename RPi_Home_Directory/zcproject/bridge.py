// PIC16F877A Configuration Bit Settings

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>


#define RUN_APPLICATION      1


/* ── MCAL ───────────────────────────────────────────────── */
#include "../SERVICES/STD_TYPES.h"
#include "../MCAL/GPIO/GPIO_interface.h"
#include "../MCAL/USART/USART_Interface.h"
#include "../MCAL/TIMER_0/TIMER_0_Interface.h"
#include "../SERVICES/DELAY.h"

/* ── HAL ────────────────────────────────────────────────── */
#include "../HAL/MOTOR/MOTOR_interface.h"
#include "../HAL/ULTRASONIC/ULTRASONIC_interface.h"
#include "../HAL/BUZZER/BUZZER_interface.h"
#include "../HAL/LCD_I2C/LCD_I2C_interface.h"
#include "../HAL/LDR/LDR_interface.h"
#include "../HAL/DOOR_SENSOR/DOOR_SENSOR_interface.h"
#include "../HAL/SWITCH/SWITCH_interface.h"
#include "../HAL/LED/LED_interface.h"

/* ── Heartbeat LED — RB3 ─────────────────────────────────
 * HB_STARTUP() : 3 flashes on power-up (GPIO alive, peripherals not yet init)
 * HB_TOGGLE()  : flips RB3 each main-loop iteration — stops blinking if frozen */
static u8 g_hb = 0U;
#define HB_TOGGLE() do { \
    g_hb ^= 1U; \
    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, g_hb ? GPIO_HIGH : GPIO_LOW); \
} while(0)

#define HB_STARTUP() do { \
    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_HIGH); DELAY_ms(150); \
    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_LOW);  DELAY_ms(150); \

    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_HIGH); DELAY_ms(150); \
    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_LOW);  DELAY_ms(150); \

    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_HIGH); DELAY_ms(150); \
    GPIO_SetPinValue(GPIO_PORTB, GPIO_PIN3, GPIO_LOW);  DELAY_ms(300); \
} while(0)

/* ── FCW zone thresholds (cm) ────────────────────────────
#define FCW_CLEAR_CM   50U   /* > this → CLEAR  zone (full speed)  */
#define FCW_WARN_CM    30U   /* > this → WARN   zone (40 %)        */
#define FCW_SLOW_CM    10U   /* > this → SLOW   zone (15 %)        */
                             /* ≤ FCW_SLOW_CM  → STOP  zone (0 %)  */

/* ── LDR hysteresis thresholds (ADC counts, 0–1023) ──────
 * Dead-band of ±50 around midpoint (512) prevents flicker. */
#define LDR_DARK_THRESHOLD    512U
#define LDR_HYST              50U
#define LDR_DARK_ENTER   (LDR_DARK_THRESHOLD + LDR_HYST)   /* 562 */
#define LDR_DARK_EXIT    (LDR_DARK_THRESHOLD - LDR_HYST)   /* 462 */

/* ── Shared volatile state (written by ISR callbacks) ──── */
static volatile u8 pretrip_ok    = 0U;  /* 1 = pre-trip checks passed        */
static volatile u8 g_lane_cmd    = 'C'; /* RPi lane cmd: 'L','C','R'         */
static volatile u8 g_door_event  = 0U;  /* 1 = door state change fired       */
static volatile u8 g_door_open   = 0U;  /* 1 = door is currently open        */
static volatile u8 g_estop_event  = 0U; /* 1 = E-stop button clicked (pulse) */
static volatile u8 g_estop_active = 0U; /* 1 = E-stop engaged (toggle state) */

/* ═══════════════════════════════════════════════════════
 *  ISR Callbacks
 *  Rule: no I2C / UART / printf inside callbacks.
 *  Only: flag writes + MOTOR_Stop() (safe in ISR).
 * ═══════════════════════════════════════════════════════ */

/* RB0 click button — toggles E-stop on/off each press */
static void EmergencyStop_Callback(void)
{
    g_estop_active ^= 1U;
    g_estop_event   = 1U;
    if (g_estop_active)
        MOTOR_Stop();
}

/* UART RX — lane commands from Raspberry Pi */
static void UART_RX_Callback(u8 cmd)
{
    if (cmd == 'L' || cmd == 'C' || cmd == 'R')
        g_lane_cmd = cmd;
}

/* Door sensor (RBIF) — POC, door 1 only */
static void Door_Callback(u8 DoorNum, u8 IsOpen)
{
    (void)DoorNum;
    g_door_open  = IsOpen;
    g_door_event = 1U;
    if (IsOpen)
    {
        MOTOR_Stop();
        pretrip_ok = 0U;
    }
}

/* ═══════════════════════════════════════════════════════
 *  FULL APPLICATION
 * ═══════════════════════════════════════════════════════ */
#if RUN_APPLICATION == 1

static void App_Print3(u16 val)
{
    if (val > 999U) val = 999U;
    LCD_I2C_PrintChar((u8)('0' + val / 100U));
    LCD_I2C_PrintChar((u8)('0' + (val / 10U) % 10U));
    LCD_I2C_PrintChar((u8)('0' + val % 10U));
}

static void Application_Init(void)
{
    GPIO_Init();
    HB_STARTUP();

    /* UART — TX telemetry, RX lane commands */
    UART_TX_Init();
    UART_RX_Init();
    UART_SetCallback(UART_RX_Callback);

    /* Timer0 + Buzzer */
    TIMER_0_Init();
    BUZZER_Init();

    /* Motors — stopped until pre-trip passes */
    MOTOR_Init();
    MOTOR_Stop();

    /* ADCON1 fix: ULTRASONIC_Init writes ADCON1=0x07 clearing ADFM.
     * Run it BEFORE LDR_Init so ADC_Init (inside LDR_Init) runs last
     * and leaves ADCON1=0x8E (ADFM=1 right-justified, PCFG=1110). */
    ULTRASONIC_Init();
    LDR_Init();

    /* LCD startup banner */
    LCD_I2C_Init();
    LCD_I2C_PrintRow(0, (const u8 *)"  ADAS  System  ");
    LCD_I2C_PrintRow(1, (const u8 *)"  Starting...   ");
    DELAY_ms(1200);

    /* SWITCH before DOOR_SENSOR: EXT_INT_Init enables PORTB pull-ups
     * and EXT_INT_Enable sets GIE — both required before RBIF fires. */
    SWITCH_Init();
    SWITCH_SetCallback(EmergencyStop_Callback);

    DOOR_SENSOR_Init();
    DOOR_SENSOR_SetCallback(Door_Callback);

    /* Enable Timer0 last — all ISR-called HAL drivers ready */
    TIMER_0_Enable();

    pretrip_ok = (!DOOR_SENSOR_IsOpen(1)) ? 1U : 0U;

    LCD_I2C_PrintRow(0, (const u8 *)"  ADAS  Ready   ");
    LCD_I2C_PrintRow(1, pretrip_ok
                        ? (const u8 *)"DOOR OK  GO!    "
                        : (const u8 *)"DOOR OPEN! WAIT ");

    UART_Write('R'); UART_Write('D'); UART_Write('Y'); UART_Write('\n');
}

static void Application_Run(void)
{
    u16 dist;
    u16 prev_dist  = 0xFFFFU;
    u8  prev_zone  = 0xFFU;
    u8  prev_lane  = 'C';
    u8  dark       = 0U;
    u8  prev_dark  = 2U;
    u8  speed      = 0U;

    while (1)
    {
        HB_TOGGLE();

        /* ── E-stop toggle event ──────────────────────── */
        if (g_estop_event)
        {
            g_estop_event = 0U;
            if (g_estop_active)
            {
                BUZZER_SingleBeep();
                LCD_I2C_PrintRow(1, (const u8 *)"** E-STOP **    ");
                UART_Write('E'); UART_Write('S'); UART_Write('T');
                UART_Write('O'); UART_Write('P'); UART_Write('\n');
            }
            else
            {
                BUZZER_Off();
                LCD_I2C_PrintRow(1, (const u8 *)"E-STOP CLEARED  ");
                UART_Write('R'); UART_Write('E'); UART_Write('S');
                UART_Write('M'); UART_Write('\n');
                DELAY_ms(500);
            }
            prev_zone = 0xFFU;
        }

        /* ── Door event ───────────────────────────────── */
        if (g_door_event)
        {
            g_door_event = 0U;
            if (g_door_open)
            {
                BUZZER_SlowBeep();
                LCD_I2C_PrintRow(1, (const u8 *)"DOOR OPEN! WAIT ");
                UART_Write('D'); UART_Write('O'); UART_Write('R');
                UART_Write(':'); UART_Write('O'); UART_Write('\n');
            }
            else
            {
                if (!DOOR_SENSOR_IsOpen(1))
                {
                    pretrip_ok = 1U;
                    BUZZER_Off();
                    LCD_I2C_PrintRow(1, (const u8 *)"DOOR OK  GO!    ");
                    UART_Write('D'); UART_Write('O'); UART_Write('R');
                    UART_Write(':'); UART_Write('C'); UART_Write('\n');
                }
                prev_zone = 0xFFU;
            }
        }

        /* ── Safety gate: door open OR E-stop engaged ─── */
        if (!pretrip_ok || g_estop_active)
        {
            MOTOR_Stop();
            DELAY_ms(200);
            continue;
        }

        /* ── F1: Ultrasonic distance ──────────────────── */
        dist = ULTRASONIC_GetDistance();

        /* ── F4: LDR + headlights ────────────────────── *
         * Hysteresis dead-band prevents headlight flicker  *
         * when ADC reading hovers near LDR_DARK_THRESHOLD. */
        {
            u16 ldr_val = LDR_Read();
            if      (!dark && ldr_val > LDR_DARK_ENTER) dark = 1U;
            else if ( dark && ldr_val < LDR_DARK_EXIT)  dark = 0U;
        }
        u8 dark_changed = (dark != prev_dark);   /* capture BEFORE updating prev_dark */
        if (dark_changed)
        {
            prev_dark = dark;
            if (dark) LDR_HeadlightsOn();
            else      LDR_HeadlightsOff();
        }

        /* ── Row 0: redraw on distance (≥2 cm) or light change ──
         * "D:NNN cm  L:LIT " — valid reading, bright
         * "D:NNN cm  L:DARK" — valid reading, dark
         * "D:--- cm  L:LIT " — Pass 2 timeout (echo stuck HIGH)
         * "NOSENSOR  L:LIT " — Pass 1 timeout (no echo at all)  */
        {
            u8 dist_changed = (dist >= ULTRASONIC_OUT_OF_RANGE)
                                ? (dist != prev_dist)
                                : ((dist > prev_dist ? dist - prev_dist
                                                     : prev_dist - dist) >= 2U);
            if (dist_changed || dark_changed)
            {
                prev_dist = dist;
                LCD_I2C_SetCursor(0, 0);
                if (dist == ULTRASONIC_NO_SENSOR)
                    LCD_I2C_Print((const u8 *)"NOSENSOR  L:");
                else if (dist >= ULTRASONIC_OUT_OF_RANGE)
                    LCD_I2C_Print((const u8 *)"D:--- cm  L:");
                else
                {
                    LCD_I2C_Print((const u8 *)"D:");
                    App_Print3(dist);
                    LCD_I2C_Print((const u8 *)" cm  L:");
                }
                LCD_I2C_Print(dark ? (const u8 *)"DARK" : (const u8 *)"LIT ");
            }
        }

        /* ── FCW zone ─────────────────────────────────── */
        u8 zone;
        if      (dist >= ULTRASONIC_OUT_OF_RANGE) zone = 0U;
        else if (dist > FCW_CLEAR_CM)             zone = 0U;
        else if (dist > FCW_WARN_CM)              zone = 1U;
        else if (dist > FCW_SLOW_CM)              zone = 2U;
        else                                      zone = 3U;

        u8 lane = g_lane_cmd;   /* snapshot — ISR may update g_lane_cmd */

        /* ── Row 1 + Motor + Buzzer ───────────────────── *
         * Redraw when zone changes OR lane changes in CLEAR zone. */
        u8 row1_changed = (zone != prev_zone) ||
                          (zone == 0U && lane != prev_lane);
        if (row1_changed)
        {
            prev_zone = zone;
            prev_lane = lane;

            switch (zone)
            {
                case 3U:
                    speed = 0U;
                    MOTOR_Stop();
                    BUZZER_Continuous();
                    LCD_I2C_PrintRow(1, (const u8 *)"[STOP  ] Spd:00%");
                    break;
                case 2U:
                    speed = 15U;
                    MOTOR_Forward(speed);
                    BUZZER_FastBeep();
                    LCD_I2C_PrintRow(1, (const u8 *)"[SLOW  ] Spd:15%");
                    break;
                case 1U:
                    speed = 40U;
                    MOTOR_Forward(speed);
                    BUZZER_SlowBeep();
                    LCD_I2C_PrintRow(1, (const u8 *)"[WARN  ] Spd:40%");
                    break;
                case 0U:
                default:
                    speed = 70U;
                    BUZZER_Off();
                    if (lane == 'L')
                    {
                        MOTOR_TurnLeft(speed);
                        LCD_I2C_PrintRow(1, (const u8 *)"[CLEAR ] <--LANE");
                    }
                    else if (lane == 'R')
                    {
                        MOTOR_TurnRight(speed);
                        LCD_I2C_PrintRow(1, (const u8 *)"[CLEAR ] LANE-->");
                    }
                    else
                    {
                        MOTOR_Forward(speed);
                        LCD_I2C_PrintRow(1, (const u8 *)"[CLEAR ] Spd:70%");
                    }
                    break;
            }
        }

        /* ── UART telemetry ───────────────────────────── *
         * Format: "D:NNN L:LIT HL:OFF S:70 N:C\n"        */
        UART_Write('D'); UART_Write(':');
        if      (dist == ULTRASONIC_NO_SENSOR)    { UART_Write('N'); UART_Write('S'); }
        else if (dist >= ULTRASONIC_OUT_OF_RANGE) { UART_Write('-'); UART_Write('-'); UART_Write('-'); }
        else                                      { UART_SendUInt(dist); }
        UART_Write(' ');
        UART_Write('L'); UART_Write(':');
        if (dark) { UART_Write('D'); UART_Write('A'); UART_Write('R'); UART_Write('K'); }
        else      { UART_Write('L'); UART_Write('I'); UART_Write('T'); }
        UART_Write(' ');
        UART_Write('H'); UART_Write('L'); UART_Write(':');
        if (dark) { UART_Write('O'); UART_Write('N'); }
        else      { UART_Write('O'); UART_Write('F'); UART_Write('F'); }
        UART_Write(' ');
        UART_Write('S'); UART_Write(':'); UART_SendUInt(speed);
        UART_Write(' ');
        UART_Write('N'); UART_Write(':'); UART_Write(lane);
        UART_Write('\n');

        DELAY_ms(150);
    }
}

int main(void)
{
    Application_Init();
    Application_Run();
    return 0;
}

#endif /* RUN_APPLICATION */