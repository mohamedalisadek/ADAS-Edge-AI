#include "ADC_interface.h"
#include "ADC_private.h"
#include "ADC_config.h"
#include "../../SERVICES/BIT_MATH.h"

void ADC_Init(void)
{
    /* Configure ADCON1: All 8 channels as analog input */
    ADCON1 = ADCON1_CONFIG;

    /* Configure ADCON0: Select clock and enable ADC */
    /* Start with channel 0 */
    u8 adcon0_value = (ADC_CLOCK_SELECT << ADCS0) | (1 << ADON);
    ADCON0 = adcon0_value;

    /* Set ADFM bit for right-justified result (10-bit) */
    SET_BIT(ADCON1, ADFM);

    /* Wait for ADC acquisition time */
    for (u16 i = 0; i < 100; i++);
}

void ADC_StartConversion(u8 Channel)
{
    /* Select channel (0-7) */
    u8 channel_bits = (Channel & 0x07);
    u8 adcon0_value = ADCON0 & 0xC3;  /* Clear channel bits */
    adcon0_value |= (channel_bits << CHS0);

    ADCON0 = adcon0_value;

    /* Start conversion */
    SET_BIT(ADCON0, GO_DONE);

    /* Wait for conversion to complete with timeout */
    u16 timeout = 0;
    while(GET_BIT(ADCON0, GO_DONE) && timeout < 1000) {
        timeout++;
    }

    /* Wait a bit for result to settle */
    for (u8 i = 0; i < 10; i++);
}

u16 ADC_GetResult(void)
{
    u16 result;

    /* Combine ADRESH (high byte) and ADRESL (low byte) */
    /* Mask to 10 bits (right-justified, ADFM=1) */
    result = (((u16)ADRESH << 8) | ADRESL) & 0x3FF;

    return result;
}

u16 ADC_Read(u8 Channel)
{
    /* Select channel and start conversion */
    ADC_StartConversion(Channel);

    /* Return the result */
    return ADC_GetResult();
}
