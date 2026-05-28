#ifndef SWITCH_INTERFACE_H
#define SWITCH_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/* Switch States */
#define SWITCH_PRESSED   1
#define SWITCH_RELEASED  0

void SWITCH_Init(void);
u8 SWITCH_GetState(void);
void SWITCH_SetCallback(void (*Callback)(void));

#endif
