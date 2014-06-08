/*
 * delay.c
 *
 *  Created on: 24.1.2012
 *      Author: strnad
 */

#include "board.h"

static volatile uint32_t msElapsed = 0;


void SysTick_Handler(void)
{
    // one tick every ms
    msElapsed++;
}

void sysTimerInit(void)
{
    SysTick_Config(SYSCLK_FREQ / 1000);
    NVIC_SetPriority(SysTick_IRQn, 15);
}

uint32_t getElapsedMs(void)
{
    uint32_t ret;
    // __disable_irq();
    ret = msElapsed;
    // __enable_irq();
    return ret;
}

uint32_t getElapsedMs2(uint32_t since)
{
    return 0;
}

void delayMs(uint16_t ms)
{
    uint32_t timeStart = getElapsedMs();
    while ((getElapsedMs() - timeStart) < ms);
}

/*
 * Delay x microseconds
 * don't exceed SysTick
 * LOAD interval (1000 us) !!!!!!
 */

void delayUs(uint16_t us)
{
    uint32_t usStart = SysTick->VAL;
    uint32_t diff;
    uint32_t tmp;
    do
    {
        tmp = SysTick->VAL;
        diff = (usStart >= tmp) ? (usStart - tmp) : ((SysTick->LOAD - tmp) + usStart);
    } while ((((1000000000UL / SYSCLK_FREQ) * diff) / 1000)  < us);
}
