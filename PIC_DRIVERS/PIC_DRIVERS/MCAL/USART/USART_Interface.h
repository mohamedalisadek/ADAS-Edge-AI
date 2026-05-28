#ifndef USART_INTERFACE_H
#define USART_INTERFACE_H

#include "USART_Private.h"
#include "USART_Config.h"
#include "../../SERVICES/STD_TYPES.h"
#include "../../SERVICES/BIT_MATH.h"

/* Initialization */
void UART_RX_Init(void);
void UART_TX_Init(void);

/* Data Operations */
void UART_Write(u8 Data);
u8   UART_Read(void);
void UART_SendString(const char *Str);  /* null-terminated ASCII string  */
void UART_SendUInt(u16 num);            /* decimal ASCII number          */

/* Status */
u8 UART_TX_Empty(void);
u8 UART_DataAvailable(void);            /* 1 = byte waiting in RX buffer */

void UART_SetCallback(void (*Callback)(u8));
void UART_SetTXCallback(void (*Callback)(void));
void UART_ISR(void);

#endif
