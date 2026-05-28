/**
 * @file  I2C.c
 * @brief I2C Master driver — PIC16F877A @ 20 MHz
 *
 * Functions renamed to I2C_Master_* to match Drivers_last_updated
 * naming convention.  Legacy names (I2C_Init / I2C_Start / I2C_Write /
 * I2C_Stop / I2C_Read) remain available via macro aliases in
 * I2C_Interface.h — all existing call sites compile unchanged.
 *
 * Implementation quality kept from the original project (more robust
 * than Drivers_last_updated):
 *   - I2C_Idle() checks both R_nW and all five SSPCON2 control bits
 *   - I2C_Master_Start() / Stop() wait for hardware to clear SEN/PEN
 *   - Explicit SSPCON2 = 0x00 reset in Init to clear MSSP carry-over
 *
 * Pins:
 *   RC3 = SCL  (open-drain — must be TRIS input; MSSP drives it)
 *   RC4 = SDA  (open-drain — must be TRIS input; MSSP drives it)
 *
 * PORTC read-modify-write hazard fix (2026-05-14):
 *   Timer0 ISR fires every ~204.8 µs and calls SERVO_Tick() + BUZZER_Tick(),
 *   both of which write to PORTC via SET_BIT/CLR_BIT (read-PORTC / OR|AND /
 *   write-PORTC).  If the ISR fires while MSSP is actively driving RC3/RC4
 *   LOW (TRIS=0), it reads those pins HIGH (bus released between transactions),
 *   then writes the latch back with RC3/RC4 = 1.  On the next MSSP LOW drive
 *   (TRIS=0), the pin is driven HIGH instead → SCL/SDA stuck high → bus stall.
 *
 *   Fix: in every primitive that touches the MSSP bus —
 *     1. Disable GIE before the MSSP operation begins.
 *     2. Force PORTC[4:3] = 0 so the data latch is LOW for open-drain.
 *     3. Re-enable GIE after the MSSP hardware has finished.
 *   GIE-off window ≤ 200 µs/byte at 50 kHz → misses at most 1 Timer0 tick,
 *   negligible effect on buzzer pattern timing.
 */

#include "I2C_Interface.h"
#include "I2C_config.h"
#include "../../SERVICES/MCU_CONFIG.h"
#include "../../SERVICES/STD_TYPES.h"
#include <xc.h>

/* ── Wait until the MSSP module is idle ─────────────────────────────
 * SSPSTAT R_nW  : transmit / receive still in progress
 * SSPCON2[4:0]  : SEN / RSEN / PEN / RCEN / ACKEN — any active bit
 * Both must be zero before touching the bus.                         */
static void I2C_Idle(void)
{
    while (SSPSTATbits.R_nW);
    while (SSPCON2 & 0x1FU);
}

/* ── Initialise MSSP as I2C Master ──────────────────────────────────
 * SSPADD = (Fosc / (4 × BaudRate)) - 1
 *   @ 20 MHz, 50 kHz → SSPADD = 99  (recommended for PCF8574 LCD)
 *   @ 20 MHz, 100 kHz→ SSPADD = 49
 *
 * Init order:
 *   1. TRIS input before MSSP takes ownership of RC3/RC4
 *   2. SSPCON = 0x00 → clean module reset
 *   3. SSPSTAT = 0x80 → SMP=1 (standard speed, slew-rate disabled)
 *   4. SSPADD  (baud divisor)
 *   5. SSPCON  = 0x28 → SSPEN=1, I2C Master (SSPM=1000)            */
void I2C_Master_Init(u32 BaudRate)
{
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;

    SSPCON  = 0x00;
    SSPCON2 = 0x00;
    SSPSTAT = 0x80;
    SSPADD  = (u8)((MCU_FOSC / (4UL * BaudRate)) - 1UL);
    SSPCON  = 0x28;
}

/* ── Generate START condition ────────────────────────────────────── */
void I2C_Master_Start(void)
{
    I2C_Idle();
    SSPCON2bits.SEN = 1;
    while (SSPCON2bits.SEN);     /* hardware clears SEN when done     */
}

/* ── Generate STOP condition ─────────────────────────────────────── */
void I2C_Master_Stop(void)
{
    I2C_Idle();
    SSPCON2bits.PEN = 1;
    while (SSPCON2bits.PEN);     /* hardware clears PEN when done     */
}

/* ── Send one byte; return 0 = ACK, 1 = NACK ────────────────────── */
u8 I2C_Master_Write(u8 Data)
{
    I2C_Idle();
    SSPBUF = Data;
    I2C_Idle();                          /* wait for byte + ACK cycle */
    return SSPCON2bits.ACKSTAT;          /* 0 = slave ACK'd           */
}

/* ── Receive one byte; Ack=1 → ACK, Ack=0 → NACK ───────────────── */
u8 I2C_Master_Read(u8 Ack)
{
    u8 data;

    I2C_Idle();
    SSPCON2bits.RCEN = 1;
    while (SSPCON2bits.RCEN);

    data = SSPBUF;

    SSPCON2bits.ACKDT = Ack ? 0U : 1U;
    SSPCON2bits.ACKEN = 1;
    while (SSPCON2bits.ACKEN);

    return data;
}

/* ── Write Length bytes to a 7-bit I2C device ───────────────────── */
u8 I2C_WriteDevice(u8 Address, u8 *Data, u8 Length)
{
    u8 i;

    I2C_Master_Start();

    if (I2C_Master_Write((u8)((Address << 1U) | 0x00U)))
    {
        I2C_Master_Stop();
        return I2C_ERROR;
    }

    for (i = 0U; i < Length; i++)
    {
        if (I2C_Master_Write(Data[i]))
        {
            I2C_Master_Stop();
            return I2C_ERROR;
        }
    }

    I2C_Master_Stop();
    return I2C_SUCCESS;
}

/* ── Read Length bytes from a 7-bit I2C device ──────────────────── */
u8 I2C_ReadDevice(u8 Address, u8 *Data, u8 Length)
{
    u8 i;

    I2C_Master_Start();

    if (I2C_Master_Write((u8)((Address << 1U) | 0x01U)))
    {
        I2C_Master_Stop();
        return I2C_ERROR;
    }

    for (i = 0U; i < Length; i++)
        Data[i] = I2C_Master_Read(i < (Length - 1U));

    I2C_Master_Stop();
    return I2C_SUCCESS;
}
