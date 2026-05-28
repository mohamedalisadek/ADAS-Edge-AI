#ifndef DOOR_SENSOR_INTERFACE_H
#define DOOR_SENSOR_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"


void DOOR_SENSOR_Init(void);
u8   DOOR_SENSOR_IsOpen(u8 DoorNum);   /* DoorNum: 1 or 2.  1=open, 0=closed */
u8   DOOR_SENSOR_AnyOpen(void);        /* 1 if either door is open            */
void DOOR_SENSOR_SetCallback(void (*Callback)(u8 DoorNum, u8 IsOpen));

/* Called from Interrupt_Manager — RBIF handler */
void DOOR_SENSOR_ISR(void);

#endif /* DOOR_SENSOR_INTERFACE_H */
