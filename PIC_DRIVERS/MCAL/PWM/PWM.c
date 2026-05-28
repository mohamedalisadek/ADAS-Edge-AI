#include "PWM_interface.h"
#include "PWM_private.h"
#include "PWM_config.h"
#include "../../SERVICES/BIT_MATH.h"
#include "../../SERVICES/MCU_CONFIG.h"
#include "../GPIO/GPIO_interface.h"

/* PWM frequency storage for duty cycle calculation */
static u16 pwm_frequency = 0;
static u16 pwm_period = 0;

void PWM_Init(u8 Channel, u16 Frequency)
{
    u8 prescaler;
    u16 pr2_value;

    /* Store frequency for later calculations */
    pwm_frequency = Frequency;

    /* Calculate PR2 value and prescaler based on frequency */
    /* PWM Period = (PR2 + 1) * 4 * Tcy * Prescaler */
    /* Tcy = 1/Fosc = 1/4MHz = 250ns */

    /* Try prescaler 1:1 first */
    pr2_value = (MCU_FOSC / (4 * Frequency)) - 1;

    if (pr2_value <= 255) {
        prescaler = PRESCALER_1;  /* 1:1 */
    }
    else if (pr2_value <= 1020) {
        prescaler = PRESCALER_4;  /* 1:4 */
        pr2_value = (MCU_FOSC / (16 * Frequency)) - 1;
    }
    else {
        prescaler = PRESCALER_16;  /* 1:16 */
        pr2_value = (MCU_FOSC / (64 * Frequency)) - 1;
    }

    pwm_period = pr2_value;

    /* Set PR2 register */
    PR2 = pr2_value;

    /* Configure Timer2 */
    T2CON = (prescaler << T2CKPS0) | (1 << TMR2ON);

    /* Set GPIO pins as outputs */
    if (Channel == PWM_CHANNEL_1) {
        GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN2, GPIO_OUTPUT);  /* RC2/CCP1 */
        CCP1CON = PWM_MODE;  /* Set PWM mode */
    }
    else if (Channel == PWM_CHANNEL_2) {
        GPIO_SetPinDirection(GPIO_PORTC, GPIO_PIN1, GPIO_OUTPUT);  /* RC1/CCP2 */
        CCP2CON = PWM_MODE;  /* Set PWM mode */
    }

    /* Initialize duty cycle to 0% */
    PWM_SetDutyCycle(Channel, 0);
}

void PWM_SetDutyCycle(u8 Channel, u8 DutyCycle)
{
    u16 pwm_value;

    if (DutyCycle > 100)
        DutyCycle = 100;

    // Calculate PWM value (16-bit): DutyCycle% * (PR2 + 1) * 4
    pwm_value = ((u32)DutyCycle * (pwm_period + 1) * 4) / 100;

    if (Channel == PWM_CHANNEL_1) {
        CCPR1L = pwm_value >> 2;      // Upper 8 bits
        CCP1CON = (CCP1CON & 0xCF) | ((pwm_value & 0x03) << 4); // Lower 2 bits -> CCP1CON<5:4>
    }
    else if (Channel == PWM_CHANNEL_2) {
        CCPR2L = pwm_value >> 2;      // Upper 8 bits
        CCP2CON = (CCP2CON & 0xCF) | ((pwm_value & 0x03) << 4); // Lower 2 bits -> CCP2CON<5:4>
    }
}

void PWM_Stop(u8 Channel)
{
    if (Channel == PWM_CHANNEL_1) {
        CCP1CON = 0;  /* Disable PWM */
    }
    else if (Channel == PWM_CHANNEL_2) {
        CCP2CON = 0;  /* Disable PWM */
    }
}
