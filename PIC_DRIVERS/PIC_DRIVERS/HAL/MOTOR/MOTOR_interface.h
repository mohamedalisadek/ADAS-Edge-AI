#ifndef MOTOR_INTERFACE_H
#define MOTOR_INTERFACE_H

#include "../../SERVICES/STD_TYPES.h"

/*
 * 4WD Motor driver — L298N dual H-bridge
 *
 *  Pin configuration lives in MOTOR_config.h.
 *  Speed is always 0–MOTOR_SPEED_MAX (%).
 *
 *  Directions:
 *    Forward   → both sides spin forward
 *    Backward  → both sides spin backward
 *    TurnLeft  → left side backward, right side forward
 *    TurnRight → right side backward, left side forward
 *    Stop      → all pins LOW, PWM = 0 %
 *
 * ⚠  OVERHEATING WARNING:
 *    Running all four motors at 100 % simultaneously caused the 5 V supply
 *    wires to overheat during hardware testing.  MOTOR_SPEED_MAX is capped
 *    at 80 % to limit peak current draw.  Do NOT raise this value without
 *    upgrading the wiring gauge and the buck-converter current rating.
 */

#define MOTOR_SPEED_MIN   0U
#define MOTOR_SPEED_MAX   80U

void MOTOR_Init(void);
void MOTOR_Forward(u8 Speed);
void MOTOR_Backward(u8 Speed);
void MOTOR_TurnLeft(u8 Speed);
void MOTOR_TurnRight(u8 Speed);
void MOTOR_Stop(void);

#endif /* MOTOR_INTERFACE_H */
