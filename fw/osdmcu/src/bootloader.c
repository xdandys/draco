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
 * @file    bootloader.c
 * @brief   bootloader code
 *
 */
#include "board.h"
#include "led.h"
#include "swtimer.h"
#include "delay.h"
#include "bootloader.h"

void HSyncInterrupt() {}
void CSyncInterrupt() {}
void VSyncInterrupt() {}

int main(void)
{
    boardInit();

    // make sure video signal can passthrough while
    // we are in bootloader
    gpioChangeMode(PinOsdMask, GPIO_Mode_OUT);
    gpioControl(PinOsdMask, 1);

    sysTimerInit();
    ledInit();

    while(1);

    return 0;
}
