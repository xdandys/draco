/*
    DRACO - Copyright (C) 2013-2014 Daniel Strnad

    This file is part of DRACO project.

    DRACO is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    DRACO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file    delay.c
 * @brief   time meassure and delay routines
 *
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
