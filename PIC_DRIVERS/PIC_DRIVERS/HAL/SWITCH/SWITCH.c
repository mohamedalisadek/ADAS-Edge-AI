#include "SWITCH_interface.h"
#include "SWITCH_private.h"
#include "../../MCAL/EXT_INT/EXT_INT_Interface.h"
#include "../../MCAL/GPIO/GPIO_interface.h"

void SWITCH_Init(void)
{
    EXT_INT_Init(); // Handles PORTB pull-ups internally
    EXT_INT_Enable();
}

u8 SWITCH_GetState(void)
{
    return !GPIO_GetPinValue(GPIO_PORTB, GPIO_PIN0);
}

void SWITCH_SetCallback(void (*Callback)(void))
{
    if(Callback != 0)
        EXT_INT_SetCallback(Callback);
}
