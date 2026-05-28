#ifndef ULTRASONIC_INTERFACE_H
#define ULTRASONIC_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"
#include "ULTRASONIC_private.h"   /* exposes ULTRASONIC_OUT_OF_RANGE = 400 */

/*
 * HC-SR04 Ultrasonic distance sensor — single sensor (F1 / FCW / AEB)
 *
 * Timing uses Timer1 (polling, no interrupt).
 * Sensor wired to RA1 (TRIG) and RA2 (ECHO).
 *
 * Return values:
 *   1–200  : measured distance in cm (valid reading)
 *   400    : ULTRASONIC_OUT_OF_RANGE — no echo / fault / object > 200 cm
 *
 * Always guard against the sentinel:
 *   if (dist >= ULTRASONIC_OUT_OF_RANGE) { treat as clear / no obstacle }
 */

void ULTRASONIC_Init(void);
u16  ULTRASONIC_GetDistance(void);

#endif /* ULTRASONIC_INTERFACE_H */
