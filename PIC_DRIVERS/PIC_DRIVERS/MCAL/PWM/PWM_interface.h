#ifndef PWM_INTERFACE_H
#define PWM_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/* PWM Channels */
#define PWM_CHANNEL_1  1
#define PWM_CHANNEL_2  2

/* PWM Duty Cycle Range: 0-100% */
#define PWM_MIN_DUTY   0
#define PWM_MAX_DUTY   100

void PWM_Init(u8 Channel, u16 Frequency);
void PWM_SetDutyCycle(u8 Channel, u8 DutyCycle);
void PWM_Stop(u8 Channel);

#endif
