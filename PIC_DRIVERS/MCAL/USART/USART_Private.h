#ifndef UART_PRIVATE_H
#define UART_PRIVATE_H

#include "../Registers_Common.h"

#define TXSTA   (*(volatile unsigned char*)0x98)
#define RCSTA   (*(volatile unsigned char*)0x18)
#define SPBRG   (*(volatile unsigned char*)0x99)
#define TXREG   (*(volatile unsigned char*)0x19)
#define RCREG   (*(volatile unsigned char*)0x1A)
#define PIE1    (*(volatile unsigned char*)0x8C)

// TXSTA Bits
#define TXEN   5
#define BRGH   2
#define SYNC   4
#define TRMT   1

// RCSTA Bits
#define SPEN   7
#define CREN   4
#define OERR   1

// PIR1 Bits
#define RCIF   5
#define TXIF   4

// PIE1 Bits
#define RCIE   5
#define TXIE   4

// INTCON Bits
#define GIE    7

#endif
