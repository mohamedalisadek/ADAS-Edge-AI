#include "LED_interface.h"
#include "../../MCAL/GPIO/GPIO_interface.h"

void LED_Init(u8 Port, u8 Pin)
{
    // Set LED pin as output
    GPIO_SetPinDirection(Port, Pin, GPIO_OUTPUT);
}

void LED_On(u8 Port, u8 Pin)
{
    // Turn LED ON
    GPIO_SetPinValue(Port, Pin, GPIO_HIGH);
}

void LED_Off(u8 Port, u8 Pin)
{
    // Turn LED OFF
    GPIO_SetPinValue(Port, Pin, GPIO_LOW);
}

void LED_Toggle(u8 Port, u8 Pin)
{
    // Toggle LED state
    u8 current = GPIO_GetPinValue(Port, Pin);

    if(current == GPIO_HIGH)
        GPIO_SetPinValue(Port, Pin, GPIO_LOW);
    else
        GPIO_SetPinValue(Port, Pin, GPIO_HIGH);
}
