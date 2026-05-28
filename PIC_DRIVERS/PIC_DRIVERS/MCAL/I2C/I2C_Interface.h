#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/* I2C Status Codes */
#define I2C_SUCCESS    1
#define I2C_ERROR      0

/* ── Primary API — matches Drivers_last_updated naming convention ───
 * BaudRate: 50000UL = 50 kHz (safe/reliable), 100000UL = 100 kHz    */
void I2C_Master_Init(u32 BaudRate);
void I2C_Master_Start(void);
void I2C_Master_Stop(void);
u8   I2C_Master_Write(u8 Data);
u8   I2C_Master_Read(u8 Ack);

/* ── Legacy aliases — backward compatible with old I2C_* call sites ─
 * Code using the old names (I2C_Init, I2C_Write, etc.) compiles
 * unchanged; both call the same underlying functions.                */
#define I2C_Init(br)    I2C_Master_Init(br)
#define I2C_Start()     I2C_Master_Start()
#define I2C_Stop()      I2C_Master_Stop()
#define I2C_Write(d)    I2C_Master_Write(d)
#define I2C_Read(a)     I2C_Master_Read(a)

/* ── Higher-level helpers ─────────────────────────────────────────── */
u8 I2C_WriteDevice(u8 Address, u8 *Data, u8 Length);
u8 I2C_ReadDevice (u8 Address, u8 *Data, u8 Length);

#endif
