#include "DOOR_SENSOR_interface.h"
#include "DOOR_SENSOR_private.h"
#include "../../MCAL/GPIO/GPIO_interface.h"
#include "../../MCAL/Registers_Common.h"
#include "../../SERVICES/BIT_MATH.h"

#define RBIF_BIT   0
#define RBIE_BIT   3

static void (*Door_Callback)(u8 DoorNum, u8 IsOpen) = 0;

/* Last known state for edge detection in ISR */
static volatile u8 door1_prev = 0;

void DOOR_SENSOR_Init(void)
{
    /* Input pin — PORTB pull-ups already enabled by EXT_INT_Init via OPTION_REG */
    GPIO_SetPinDirection(DOOR1_PORT, DOOR1_PIN, GPIO_INPUT);

    /* Indicator LED pin as output, start OFF */
    GPIO_SetPinDirection(DOOR1_LED_PORT, DOOR1_LED_PIN, GPIO_OUTPUT);
    GPIO_SetPinValue(DOOR1_LED_PORT, DOOR1_LED_PIN, GPIO_LOW);

    /* Snapshot initial state */
    door1_prev = GPIO_GetPinValue(DOOR1_PORT, DOOR1_PIN);

    /* Enable PORTB change interrupt (RBIE) and clear its flag.
     * GIE is already set by EXT_INT_Enable() called from SWITCH_Init(). */
    CLR_BIT(INTCON, RBIF_BIT);
    SET_BIT(INTCON, RBIE_BIT);
}

u8 DOOR_SENSOR_IsOpen(u8 DoorNum)
{
    (void)DoorNum;   /* only door 1 is used */
    return (GPIO_GetPinValue(DOOR1_PORT, DOOR1_PIN) == DOOR_OPEN) ? 1U : 0U;
}

u8 DOOR_SENSOR_AnyOpen(void)
{
    return DOOR_SENSOR_IsOpen(1);
}

void DOOR_SENSOR_SetCallback(void (*Callback)(u8 DoorNum, u8 IsOpen))
{
    if (Callback != 0)
        Door_Callback = Callback;
}

/* Called from RBIF handler in Interrupt_Manager.c */
void DOOR_SENSOR_ISR(void)
{
    /* Reading PORTB is required to clear the mismatch that triggered RBIF */
    u8 door1_now = GPIO_GetPinValue(DOOR1_PORT, DOOR1_PIN);

    if (door1_now != door1_prev)
    {
        door1_prev = door1_now;
        /* Update indicator LED */
        GPIO_SetPinValue(DOOR1_LED_PORT, DOOR1_LED_PIN,
                         (door1_now == DOOR_OPEN) ? GPIO_HIGH : GPIO_LOW);
        if (Door_Callback != 0)
            Door_Callback(1, (door1_now == DOOR_OPEN) ? 1U : 0U);
    }
}
