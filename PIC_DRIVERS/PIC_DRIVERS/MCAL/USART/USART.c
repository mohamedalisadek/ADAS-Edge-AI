#include "USART_Interface.h"
#include "../GPIO/GPIO_interface.h"

void (*UART_RX_Callback)(u8) = 0;
void (*UART_TX_Callback)(void) = 0;

void UART_RX_Init(void)
{
    SET_BIT(TXSTA, BRGH);
    SPBRG = (FOSC / (16UL * UART_BAUDRATE)) - 1;
    CLR_BIT(TXSTA, SYNC);
    SET_BIT(RCSTA, SPEN);

    GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN6, GPIO_OUTPUT);  // RC6/TX
    GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN7, GPIO_INPUT);  // RC7/RX

    SET_BIT(RCSTA, CREN);
    SET_BIT(PIE1, RCIE);
    SET_BIT(INTCON, PEIE);
    SET_BIT(INTCON, GIE);
}

void UART_TX_Init(void)
{
    SET_BIT(TXSTA, BRGH);
    SPBRG = (FOSC / (16UL * UART_BAUDRATE)) - 1;
    CLR_BIT(TXSTA, SYNC);
    SET_BIT(RCSTA, SPEN);

    GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN6, GPIO_OUTPUT);  // RC6/TX
    GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN7, GPIO_INPUT);  // RC7/RX

    SET_BIT(TXSTA, TXEN);
    /* NOTE: TXIE is intentionally NOT enabled here.
     * We use polled TX (UART_Write waits for TRMT).
     * Enabling TXIE would cause a continuous interrupt
     * storm whenever TXREG is empty, starving the main loop. */
    SET_BIT(INTCON, PEIE);
    SET_BIT(INTCON, GIE);
}

void UART_Write(u8 Data)
{
    while(!GET_BIT(TXSTA, TRMT));
    TXREG = Data;
}

u8 UART_Read(void)
{
    while(!GET_BIT(PIR1, RCIF));

    if(GET_BIT(RCSTA, OERR)) { // Check RX overflow
        CLR_BIT(RCSTA, CREN);
        SET_BIT(RCSTA, CREN);
    }

    return RCREG;
}

u8 UART_TX_Empty(void)
{
    return GET_BIT(TXSTA, TRMT);
}

void UART_SendString(const char *Str)
{
    while (*Str)
    {
        UART_Write((u8)*Str);
        Str++;
    }
}

/**
 * @brief Send u16 as decimal ASCII over UART.
 *        Ported from Drivers_last_updated — avoids a local copy in main.c.
 */
void UART_SendUInt(u16 num)
{
    char buf[6];
    u8   i = 0U;
    if (num == 0U) { UART_Write('0'); return; }
    while (num > 0U) { buf[i++] = (char)('0' + (num % 10U)); num /= 10U; }
    while (i > 0U)   { UART_Write((u8)buf[--i]); }
}

u8 UART_DataAvailable(void)
{
    return GET_BIT(PIR1, RCIF);
}

void UART_SetCallback(void (*Callback)(u8))
{
    if(Callback != 0)
        UART_RX_Callback = Callback;
}

void UART_SetTXCallback(void (*Callback)(void))
{
    if(Callback != 0)
        UART_TX_Callback = Callback;
}

void UART_ISR(void)
{
    if(GET_BIT(PIR1, RCIF)) {
        u8 data = RCREG;
        if(UART_RX_Callback != 0)
            UART_RX_Callback(data);
    }

    if(GET_BIT(PIR1, TXIF)) {
        if(UART_TX_Callback != 0)
            UART_TX_Callback();
    }
}