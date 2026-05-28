#ifndef LDR_PRIVATE_H
#define LDR_PRIVATE_H

#include "../../MCAL/ADC/ADC_interface.h"

/* LDR connected to AN0 / RA0 */
#define LDR_ADC_CHANNEL     ADC_CHANNEL_0

/*
 * Darkness threshold (0–1023).
 * ADC reads LOW when bright (low resistance), HIGH when dark.
 * Tune this value in hardware testing.
 * Below threshold = bright, above threshold = dark.
 */
#define LDR_DARK_THRESHOLD  512U

/* Headlight LED pin configuration — passed to LED driver, not used directly.
 * To relocate the headlights, change only these four defines. */
#include "../../MCAL/GPIO/GPIO_interface.h"   /* for GPIO_PORTx / GPIO_PINx constants */
#include "../LED/LED_interface.h"              /* LED abstraction layer                */
#define HEADLIGHT_PORT      GPIO_PORTD
#define HEADLIGHT_PIN1      GPIO_PIN4
#define HEADLIGHT_PIN2      GPIO_PIN5
#define HEADLIGHT_PIN3      GPIO_PIN6
#define HEADLIGHT_PIN4      GPIO_PIN7

#endif /* LDR_PRIVATE_H */
