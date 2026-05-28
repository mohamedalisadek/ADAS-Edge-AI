#include "EXT_INT_interface.h"
#include "EXT_INT_Private.h"
#include "../../SERVICES/BIT_MATH.h"

static void (*EXT_INT_Callback)(void) = 0;

void EXT_INT_Init(void)
{
    GPIO_SetPinDirection(EXT_INT_PORT, EXT_INT_PIN, GPIO_INPUT);
    EXT_INT_SetEdge(falling_edge);
    CLR_BIT(INTCON, INTF_BIT);
    CLR_BIT(INTCON, INTE_BIT);
    CLR_BIT(OPTION_REG, 7);  // Enable PORTB pull-ups (RBPU=0)
}

void EXT_INT_Enable(void)
{
    CLR_BIT(INTCON, INTF_BIT);
    SET_BIT(INTCON, INTE_BIT);
    SET_BIT(INTCON, GIE_BIT);
}

void EXT_INT_Disable(void)
{
    CLR_BIT(INTCON, INTE_BIT);
    CLR_BIT(INTCON, INTF_BIT);
}

void EXT_INT_SetEdge(u8 Edgetype)
{
    if(Edgetype == rising_edge)
        SET_BIT(OPTION_REG, INTEDG_BIT);   /* 1=Rising - was inverted */
    else
        CLR_BIT(OPTION_REG, INTEDG_BIT);   /* 0=Falling - was inverted */
}

void EXT_INT_SetCallback(void (*ptr)(void))
{
    if(ptr != 0)
        EXT_INT_Callback = ptr;
}

void EXT_INT_ISR(void)
{
    CLR_BIT(INTCON, INTF_BIT);
    if(EXT_INT_Callback != 0)
        EXT_INT_Callback();
}
/* void interrupt() lives in Interrupt_Manager.c only */