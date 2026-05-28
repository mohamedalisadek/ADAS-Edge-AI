#ifndef DOOR_SENSOR_PRIVATE_H
#define DOOR_SENSOR_PRIVATE_H

#include "../../MCAL/GPIO/GPIO_interface.h"

/*
 * Magnetic reed switch on RB4 (PORTB change interrupt — RBIF).
 * Reed switch: CLOSED (pin LOW) when door is shut, OPEN (pin HIGH) when open.
 * PORTB internal pull-ups are enabled so open-circuit = HIGH.
 */
#define DOOR1_PORT      GPIO_PORTB
#define DOOR1_PIN       GPIO_PIN4

#define DOOR_CLOSED     0U   /* pin LOW  → door shut  */
#define DOOR_OPEN       1U   /* pin HIGH → door open  */

/* Indicator LED (driven by PORTB output) */
#define DOOR1_LED_PORT  GPIO_PORTB
#define DOOR1_LED_PIN   GPIO_PIN1   /* Red LED — Door 1 fail */

#endif /* DOOR_SENSOR_PRIVATE_H */
