#include "LDR_interface.h"
#include "LDR_private.h"       /* brings in LED_interface.h + pin config  */
#include "../../MCAL/ADC/ADC_interface.h"

/* Note: GPIO is NOT included directly here.
 * All LED output goes through the LED abstraction layer (LED_Init/On/Off).
 * GPIO_PORTx / GPIO_PINx constants come from LDR_private.h (config only). */

void LDR_Init(void)
{
    /* ADC — initialise sensor channel */
    ADC_Init();

    /* Headlight LEDs — initialised through LED driver (sets pins as outputs) */
    LED_Init(HEADLIGHT_PORT, HEADLIGHT_PIN1);
    LED_Init(HEADLIGHT_PORT, HEADLIGHT_PIN2);
    LED_Init(HEADLIGHT_PORT, HEADLIGHT_PIN3);
    LED_Init(HEADLIGHT_PORT, HEADLIGHT_PIN4);

    LDR_HeadlightsOff();   /* ensure all headlights start OFF */
}

u16 LDR_Read(void)
{
    return ADC_Read(LDR_ADC_CHANNEL);
}

u8 LDR_IsDark(void)
{
    return (LDR_Read() > LDR_DARK_THRESHOLD) ? 1U : 0U;
}

void LDR_HeadlightsOn(void)
{
    LED_On(HEADLIGHT_PORT, HEADLIGHT_PIN1);
    LED_On(HEADLIGHT_PORT, HEADLIGHT_PIN2);
    LED_On(HEADLIGHT_PORT, HEADLIGHT_PIN3);
    LED_On(HEADLIGHT_PORT, HEADLIGHT_PIN4);
}

void LDR_HeadlightsOff(void)
{
    LED_Off(HEADLIGHT_PORT, HEADLIGHT_PIN1);
    LED_Off(HEADLIGHT_PORT, HEADLIGHT_PIN2);
    LED_Off(HEADLIGHT_PORT, HEADLIGHT_PIN3);
    LED_Off(HEADLIGHT_PORT, HEADLIGHT_PIN4);
}
