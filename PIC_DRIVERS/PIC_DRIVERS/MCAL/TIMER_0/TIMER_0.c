#include "TIMER_0_Interface.h"
#include "TIMER_0_Private.h"
#include "TIMER_0_Config.h"
#include "../../SERVICES/BIT_MATH.h"

static void (*TIMER_0_Callback)(void) = 0;  /* static - not accessible outside */

void TIMER_0_Init(void)
{
    /* Clear Timer0 counter */
    TMR0 = 0x00;

    /* Configure prescaler assignment */
    CLR_BIT(OPTION_REG, PSA_BIT);   /* Assign prescaler to Timer0 */

    /* Configure clock source based on mode */
#if (TIMER_0_MODE == TIMER_0_MODE_TIMER)
    CLR_BIT(OPTION_REG, T0CS_BIT);  /* Clock source = FOSC/4 (internal clock) */
#else
    SET_BIT(OPTION_REG, T0CS_BIT);  /* Clock source = RA4/T0CKI (external clock) */
#endif

    /* Configure prescaler bits */
#if (TIMER_0_PRESCALER == TIMER_0_PRESCALER_2)
    CLR_BIT(OPTION_REG, PS0_BIT);
    CLR_BIT(OPTION_REG, PS1_BIT);
    CLR_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_4)
    SET_BIT(OPTION_REG, PS0_BIT);
    CLR_BIT(OPTION_REG, PS1_BIT);
    CLR_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_8)
    CLR_BIT(OPTION_REG, PS0_BIT);
    SET_BIT(OPTION_REG, PS1_BIT);
    CLR_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_16)
    SET_BIT(OPTION_REG, PS0_BIT);
    SET_BIT(OPTION_REG, PS1_BIT);
    CLR_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_32)
    CLR_BIT(OPTION_REG, PS0_BIT);
    CLR_BIT(OPTION_REG, PS1_BIT);
    SET_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_64)
    SET_BIT(OPTION_REG, PS0_BIT);
    CLR_BIT(OPTION_REG, PS1_BIT);
    SET_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_128)
    CLR_BIT(OPTION_REG, PS0_BIT);
    SET_BIT(OPTION_REG, PS1_BIT);
    SET_BIT(OPTION_REG, PS2_BIT);
#elif (TIMER_0_PRESCALER == TIMER_0_PRESCALER_256)
    SET_BIT(OPTION_REG, PS0_BIT);
    SET_BIT(OPTION_REG, PS1_BIT);
    SET_BIT(OPTION_REG, PS2_BIT);
#endif

    /* Clear Timer0 interrupt flag */
    CLR_BIT(INTCON, T0IF_BIT);
}

void TIMER_0_Enable(void)
{
    /* Clear Timer0 interrupt flag */
    CLR_BIT(INTCON, T0IF_BIT);

    /* Enable Timer0 interrupt */
    SET_BIT(INTCON, T0IE_BIT);

    /* Enable global interrupts */
    SET_BIT(INTCON, GIE_BIT);
}

void TIMER_0_Disable(void)
{
    /* Disable Timer0 interrupt */
    CLR_BIT(INTCON, T0IE_BIT);

    /* Clear Timer0 interrupt flag */
    CLR_BIT(INTCON, T0IF_BIT);
}

void TIMER_0_SetCallback(void (*ptr)(void))
{
    if(ptr != 0)
        TIMER_0_Callback = ptr;
}

void TIMER_0_ISR(void)
{
    /* Clear Timer0 interrupt flag */
    CLR_BIT(INTCON, T0IF_BIT);

    /* Call user callback if set */
    if(TIMER_0_Callback != 0)
        TIMER_0_Callback();
}
