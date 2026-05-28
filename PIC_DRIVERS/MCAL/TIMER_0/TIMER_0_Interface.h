#ifndef TIMER_0_INTERFACE_H
#define TIMER_0_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

void TIMER_0_Init(void);
void TIMER_0_Enable(void);
void TIMER_0_Disable(void);
void TIMER_0_SetCallback(void (*ptr)(void));
void TIMER_0_ISR(void);

#endif