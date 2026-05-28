#ifndef LDR_INTERFACE_H
#define LDR_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/*
 * LDR ambient-light sensor driver (F4 — Auto Headlights)
 *
 * Reads ambient light via ADC and exposes headlight control.
 * Headlight output is routed through the LED driver — pin
 * assignments are internal to LDR_private.h (not this interface).
 */

void LDR_Init(void);
u16  LDR_Read(void);           /* raw ADC value 0–1023               */
u8   LDR_IsDark(void);         /* 1 = dark (headlights should be on) */
void LDR_HeadlightsOn(void);
void LDR_HeadlightsOff(void);

#endif /* LDR_INTERFACE_H */
