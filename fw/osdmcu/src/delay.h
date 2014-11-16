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
 * @file    delay.h
 * @brief   time meassure and delay routines
 *
 */
#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>

#include "board.h"

static inline uint32_t ticksToNs(uint32_t ticks)
{
    return (((uint64_t)ticks * 1000ULL) / (SYSCLK_FREQ / 1000000));
}

static inline uint32_t tickIntervalNs(uint32_t ticks1, uint32_t ticks2)
{
    return ticksToNs(ticks2 - ticks1);
}
static inline uint32_t getTick(void) {return DWT->CYCCNT;}

void sysTimerInit(void);
uint32_t getElapsedMs(void);
uint32_t getElapsedMs2(uint32_t since);
void delayMs(uint16_t ms);
void delayUs(uint16_t us);

#endif /* DELAY_H_ */
