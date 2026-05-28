#include "MOTOR_interface.h"
#include "MOTOR_private.h"
#include "MOTOR_config.h"
#include "../../MCAL/GPIO/GPIO_interface.h"
#include "../../MCAL/PWM/PWM_interface.h"

/* ── private helpers ──────────────────────────────────────── */

static void MotorA_Set(u8 Direction, u8 Speed)
{
    if (Speed > MOTOR_SPEED_MAX) Speed = MOTOR_SPEED_MAX;

    if (Direction == MOTOR_DIR_FORWARD) {
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN, GPIO_HIGH);
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN, GPIO_LOW);
    }
    else if (Direction == MOTOR_DIR_BACKWARD) {
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN, GPIO_LOW);
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN, GPIO_HIGH);
    }
    else {
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN, GPIO_LOW);
        GPIO_SetPinValue(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN, GPIO_LOW);
        Speed = 0;
    }
    PWM_SetDutyCycle(MOTOR_A_PWM_CH, Speed);
}

static void MotorB_Set(u8 Direction, u8 Speed)
{
    if (Speed > MOTOR_SPEED_MAX) Speed = MOTOR_SPEED_MAX;

    if (Direction == MOTOR_DIR_FORWARD) {
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN, GPIO_HIGH);
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN, GPIO_LOW);
    }
    else if (Direction == MOTOR_DIR_BACKWARD) {
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN, GPIO_LOW);
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN, GPIO_HIGH);
    }
    else {
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN, GPIO_LOW);
        GPIO_SetPinValue(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN, GPIO_LOW);
        Speed = 0;
    }
    PWM_SetDutyCycle(MOTOR_B_PWM_CH, Speed);
}

/* ── public API ───────────────────────────────────────────── */

void MOTOR_Init(void)
{
    GPIO_SetPinDirection(MOTOR_A_DIR_PORT, MOTOR_A_IN1_PIN, GPIO_OUTPUT);
    GPIO_SetPinDirection(MOTOR_A_DIR_PORT, MOTOR_A_IN2_PIN, GPIO_OUTPUT);
    GPIO_SetPinDirection(MOTOR_B_DIR_PORT, MOTOR_B_IN1_PIN, GPIO_OUTPUT);
    GPIO_SetPinDirection(MOTOR_B_DIR_PORT, MOTOR_B_IN2_PIN, GPIO_OUTPUT);

    PWM_Init(MOTOR_A_PWM_CH, MOTOR_PWM_FREQ);
    PWM_Init(MOTOR_B_PWM_CH, MOTOR_PWM_FREQ);

    MOTOR_Stop();
}

void MOTOR_Forward(u8 Speed)
{
    MotorA_Set(MOTOR_DIR_FORWARD, Speed);
    MotorB_Set(MOTOR_DIR_FORWARD, Speed);
}

void MOTOR_Backward(u8 Speed)
{
    MotorA_Set(MOTOR_DIR_BACKWARD, Speed);
    MotorB_Set(MOTOR_DIR_BACKWARD, Speed);
}

void MOTOR_TurnLeft(u8 Speed)
{
    /* Pivot turn: left back, right forward */
    MotorA_Set(MOTOR_DIR_BACKWARD, Speed);
    MotorB_Set(MOTOR_DIR_FORWARD,  Speed);
}

void MOTOR_TurnRight(u8 Speed)
{
    /* Pivot turn: right back, left forward */
    MotorA_Set(MOTOR_DIR_FORWARD,  Speed);
    MotorB_Set(MOTOR_DIR_BACKWARD, Speed);
}

void MOTOR_Stop(void)
{
    MotorA_Set(MOTOR_DIR_STOP, 0);
    MotorB_Set(MOTOR_DIR_STOP, 0);
}
