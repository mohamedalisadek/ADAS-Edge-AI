#ifndef ADC_INTERFACE_H
#define ADC_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/* ADC Channels (AN0-AN7) */
#define ADC_CHANNEL_0  0
#define ADC_CHANNEL_1  1
#define ADC_CHANNEL_2  2
#define ADC_CHANNEL_3  3
#define ADC_CHANNEL_4  4
#define ADC_CHANNEL_5  5
#define ADC_CHANNEL_6  6
#define ADC_CHANNEL_7  7

/* ADC Resolution */
#define ADC_RESOLUTION_10BIT  10  /* 0-1023 */
#define ADC_RESOLUTION_8BIT   8   /* 0-255 */

void ADC_Init(void);
u16 ADC_Read(u8 Channel);
void ADC_StartConversion(u8 Channel);
u16 ADC_GetResult(void);

#endif
