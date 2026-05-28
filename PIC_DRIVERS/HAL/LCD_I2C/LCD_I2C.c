/**
 * @file  LCD_I2C.c
 * @brief HD44780 16×2 LCD via PCF8574 I2C backpack — PIC16F877A
 *
 * Merged improvements from Drivers_last_updated/HAL/LCD (Ahmed Salah, 2026-04-23):
 *
 *   1. I2C speed lowered to 50 kHz
 *      Cheap PCF8574 modules often have excessive bus capacitance; 100 kHz
 *      can corrupt nibble transfers and produce a working cursor (from the
 *      DISPLAY_ON command) but garbled/invisible character writes.
 *
 *   2. Proper HD44780 init sequence (datasheet Figure 24)
 *      Added LCD_CMD_DISPLAY_OFF before LCD_CMD_CLEAR, which the datasheet
 *      requires — omitting it can leave the display in an undefined state
 *      after power-on.
 *
 *   3. DISPLAY_ON = 0x0C (cursor off, no blink)
 *      0x0F blinks the cursor on top of the first character, visually
 *      masking it.  0x0C is the production-correct value.
 *
 *   4. Explicit D4–D7 bit mapping in LCD_SendNibble
 *      Each data bit is shifted to its exact PCF8574 pin position by name,
 *      making the mapping self-documenting and immune to upper/lower nibble
 *      confusion.
 *
 *   5. Stack-saving macros for PCF8574_Write and LCD_Pulse_EN
 *      PIC16F877A has an 8-level hardware call stack.  Replacing the two
 *      innermost helpers with macros frees two stack levels for LCD_I2C_Print
 *      and the application caller.
 *
 * Public API is unchanged — all existing call sites in main.c compile as-is.
 */

#include "LCD_I2C_interface.h"
#include "LCD_I2C_private.h"
#include "../../MCAL/I2C/I2C_interface.h"
#include "../../SERVICES/MCU_CONFIG.h"   /* defines _XTAL_FREQ for __delay_ms / __delay_us */
#include "../../SERVICES/STD_TYPES.h"
#include <xc.h>

/* Backlight state — default ON */
static u8 g_bl = LCD_BL_ON;

/* ── PCF8574 single-byte write ──────────────────────────────────────
 * Implemented as a macro to save two precious PIC16 call-stack levels.
 * Sequence: START → address+W → data byte → STOP
 *
 * PORTC read-modify-write hazard protection:
 *   Timer0 ISR (~204.8 µs) writes to PORTC via SET_BIT/CLR_BIT.
 *   If it fires while MSSP holds SDA LOW (after START, TRIS=0, latch
 *   must be 0), the ISR read-modify-writes latch to 1.  When MSSP next
 *   asserts TRIS=0 the pin is driven HIGH → SCL/SDA stuck → bus stall.
 *
 *   Disabling GIE per-primitive is NOT enough: GIE re-enables between
 *   I2C_Master_Start() and I2C_Master_Write(), and an ISR firing in that
 *   window creates a phantom STOP while SDA should still be held LOW.
 *
 *   Fix: one GIE-off block spans the entire Start→Write→Write→Stop.
 *   Force RC3=RC4 latch LOW before the first bus operation.
 *   Window ≈ 3 bytes × 200 µs at 50 kHz ≈ 600 µs (~3 Timer0 ticks).
 *   Buzzer pattern is negligibly affected.                             */
#define PCF8574_Write(d) do {                               \
    INTCONbits.GIE = 0;                                     \
    PORTC &= ~0x18U;             /* RC3=RC4 latch = 0       */         \
    I2C_Master_Start();                                     \
    I2C_Master_Write((u8)((LCD_I2C_ADDRESS << 1U) | 0U));  \
    I2C_Master_Write((u8)(d));                              \
    I2C_Master_Stop();                                      \
    INTCONbits.GIE = 1;                                     \
} while (0)

/* ── Pulse the Enable pin to latch one nibble ───────────────────────
 * HD44780 requires E high for ≥ 450 ns; 2 µs is safe.
 * 50 µs settling after E falls before the next nibble.              */
#define LCD_Pulse_EN(b) do {                                \
    PCF8574_Write((b) | (1U << LCD_EN_BIT));                \
    __delay_us(2);                                          \
    PCF8574_Write((b) & ~(1U << LCD_EN_BIT));               \
    __delay_us(50);                                         \
} while (0)

/* ── Send one 4-bit nibble to the LCD ──────────────────────────────
 * nibble : lower 4 bits are the data (D7..D4 value, NOT pre-shifted)
 * rs     : 1 = data register, 0 = command register
 *
 * Each data bit is mapped individually to its named PCF8574 pin so
 * the bit assignment is explicit and can be audited against the
 * LCD_I2C_private.h wiring table.                                   */
static void LCD_SendNibble(u8 nibble, u8 rs)
{
    u8 b = g_bl;                            /* start with backlight state */
    if (rs)            b |= (1U << LCD_RS_BIT);
    if (nibble & 0x1U) b |= (1U << LCD_D4_BIT);
    if (nibble & 0x2U) b |= (1U << LCD_D5_BIT);
    if (nibble & 0x4U) b |= (1U << LCD_D6_BIT);
    if (nibble & 0x8U) b |= (1U << LCD_D7_BIT);
    LCD_Pulse_EN(b);
}

/* ── Send a full byte in two nibbles (high nibble first) ────────────
 * Clear and Home commands need 2 ms to execute; all others 50 µs.  */
static void LCD_SendByte(u8 byte, u8 rs)
{
    LCD_SendNibble((byte >> 4U) & 0x0FU, rs);  /* high nibble */
    LCD_SendNibble( byte        & 0x0FU, rs);  /* low  nibble */

    if (byte == LCD_CMD_CLEAR || byte == LCD_CMD_HOME)
        __delay_ms(2);
    else
        __delay_us(50);
}

static void LCD_Command(u8 cmd) { LCD_SendByte(cmd, 0); }
static void LCD_Data(u8 ch)     { LCD_SendByte(ch,  1); }

/* ══════════════════════════════════════════════════════════════════ */
/*  Public API                                                        */
/* ══════════════════════════════════════════════════════════════════ */

void LCD_I2C_Init(void)
{
    /* 50 kHz is recommended for PCF8574 backpack modules.
     * Cheap boards have high bus capacitance that corrupts nibble
     * transfers at 100 kHz — characters invisible, cursor still shows. */
    I2C_Master_Init(50000UL);
    __delay_ms(50);

    /* Ensure the PCF8574 sees a clean byte so it wakes up properly */
    PCF8574_Write(g_bl);
    __delay_ms(5);

    /* HD44780 4-bit initialisation sequence — datasheet Figure 24
     *
     * Three "Function Set" nibbles (0x3) in 8-bit mode force the
     * controller into a known state regardless of its power-on mode.
     * Then nibble 0x2 switches it to 4-bit mode.                    */
    LCD_SendNibble(0x03U, 0); __delay_ms(5);    /* 8-bit attempt 1  */
    LCD_SendNibble(0x03U, 0); __delay_ms(5);    /* 8-bit attempt 2  */
    LCD_SendNibble(0x03U, 0); __delay_us(200);  /* 8-bit attempt 3  */
    LCD_SendNibble(0x02U, 0); __delay_us(200);  /* switch to 4-bit  */

    LCD_Command(LCD_CMD_4BIT_2LINE);   /* 4-bit, 2 lines, 5×8 font   */
    LCD_Command(LCD_CMD_DISPLAY_OFF);  /* display OFF before clear    */
    LCD_Command(LCD_CMD_CLEAR);        /* clear DDRAM (needs 2 ms)    */
    __delay_ms(2);
    LCD_Command(LCD_CMD_ENTRY_MODE);   /* cursor right, no shift      */
    LCD_Command(LCD_CMD_DISPLAY_ON);   /* display ON, cursor off      */
}

void LCD_I2C_Clear(void)
{
    LCD_Command(LCD_CMD_CLEAR);
    __delay_ms(2);
}

void LCD_I2C_SetCursor(u8 Row, u8 Col)
{
    u8 addr = (Row == 0U) ? (LCD_ROW0_ADDR + Col)
                          : (LCD_ROW1_ADDR + Col);
    LCD_Command(LCD_CMD_SET_DDRAM | addr);
}

void LCD_I2C_PrintChar(u8 Ch)
{
    LCD_Data(Ch);
}

void LCD_I2C_Print(const u8 *Str)
{
    while (*Str) { LCD_Data(*Str++); }
}

void LCD_I2C_PrintNumber(s16 Num)
{
    u8  buf[7];   /* '-32768\0' worst case */
    u8  i   = 0;
    u16 val;

    if (Num < 0) { LCD_Data('-'); val = (u16)(-Num); }
    else         { val = (u16)Num; }

    if (val == 0U) { LCD_Data('0'); return; }

    while (val > 0U)
    {
        buf[i++] = (u8)('0' + (val % 10U));
        val /= 10U;
    }
    while (i > 0U) { LCD_Data(buf[--i]); }
}

void LCD_I2C_PrintRow(u8 Row, const u8 *Str)
{
    u8 col;
    LCD_I2C_SetCursor(Row, 0U);
    for (col = 0U; col < LCD_COLS && Str[col]; col++)
        LCD_Data(Str[col]);
    /* Pad remainder with spaces to erase any leftover characters */
    for (; col < LCD_COLS; col++)
        LCD_Data(' ');
}

void LCD_I2C_SetBacklight(u8 state)
{
    g_bl = state ? LCD_BL_ON : 0U;
    PCF8574_Write(g_bl);
}
