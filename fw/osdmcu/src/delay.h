/*
 * delay.h
 *
 *  Created on: 24.1.2012
 *      Author: strnad
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

/*
* Delay x microseconds
 * don't exceed SysTick
 * LOAD interval (1000 us) !!!!!!
 * meassured 4 % error
 */
void delayUs(uint16_t us);



#endif /* DELAY_H_ */
