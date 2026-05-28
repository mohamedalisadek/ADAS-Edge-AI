#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/*
 * L298N wiring for 4-wheel-drive car
 * ────────────────────────────────────────────────────────────────
 *  Left  motors (FL + BL wired in parallel) → L298N Channel A
 *  Right motors (FR + BR wired in parallel) → L298N Channel B
 *
 *  ENA (Motor A enable / speed) → RC2 / CCP1  (hardware PWM ch1)
 *  ENB (Motor B enable / speed) → RC1 / CCP2  (hardware PWM ch2)
 *
 *  IN1 / IN2  control Motor A direction → PORTD pins 0, 1
 *  IN3 / IN4  control Motor B direction → PORTD pins 2, 3
 * ────────────────────────────────────────────────────────────────
 */

#include "../../MCAL/GPIO/GPIO_interface.h"
#include "../../MCAL/PWM/PWM_interface.h"

/* Motor A — Left side */
#define MOTOR_A_DIR_PORT    GPIO_PORTD
#define MOTOR_A_IN1_PIN     GPIO_PIN0
#define MOTOR_A_IN2_PIN     GPIO_PIN1
#define MOTOR_A_PWM_CH      PWM_CHANNEL_1   /* RC2 / CCP1 */

/* Motor B — Right side */
#define MOTOR_B_DIR_PORT    GPIO_PORTD
#define MOTOR_B_IN1_PIN     GPIO_PIN2
#define MOTOR_B_IN2_PIN     GPIO_PIN3
#define MOTOR_B_PWM_CH      PWM_CHANNEL_2   /* RC1 / CCP2 */

/* PWM frequency for motor control (5 kHz — quiet and efficient) */
#define MOTOR_PWM_FREQ      5000U

#endif /* MOTOR_CONFIG_H */
