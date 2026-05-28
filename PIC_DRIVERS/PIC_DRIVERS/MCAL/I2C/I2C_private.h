#ifndef I2C_PRIVATE_H
#define I2C_PRIVATE_H

#include "../../SERVICES/STD_TYPES.h"

/* PIC16F877A I2C/MSSP Registers */
#define SSPCON      (*(volatile u8*)0x14)   /* SSPCON1  */
#define SSPCON2     (*(volatile u8*)0x91)   /* SSPCON2 - was missing entirely */
#define SSPSTAT     (*(volatile u8*)0x94)
#define SSPADD      (*(volatile u8*)0x93)
#define SSPBUF      (*(volatile u8*)0x13)

/* SSPCON1 Bits */
#define SSPM0       0
#define SSPM1       1
#define SSPM2       2
#define SSPM3       3
#define CKP         4
#define SSPEN       5

/* SSPCON2 Bits (were missing — these are the I2C control bits) */
#define SEN         0   /* Start Enable                  */
#define RSEN        1   /* Repeated Start Enable         */
#define PEN         2   /* Stop Enable                   */
#define RCEN        3   /* Receive Enable                */
#define ACKEN       4   /* Acknowledge Sequence Enable   */
#define ACKDT       5   /* Acknowledge Data (0=ACK,1=NACK) */
#define ACKSTAT     6   /* Acknowledge Status (0=ACK from slave) */

/* SSPSTAT Bits */
#define BF          0   /* Buffer Full      */
#define UA          1
#define RW          2   /* R/W bit (1=read, 0=write) */
#define S_BIT       3   /* Start bit detected        */
#define P_BIT       4   /* Stop bit detected         */
#define DA          5   /* Data/Address              */

/* I2C Master Mode configuration value for SSPCON1
 * SSPEN=1 (bit5) | SSPM=1000 (bits3:0) -> 0x28 */
#define I2C_MASTER_MODE  0x28

#endif