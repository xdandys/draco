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
#include "flashupdate.h"
#include "debug.h"

void HSyncInterrupt() {}
void CSyncInterrupt() {}
void VSyncInterrupt() {}

static void bootFirmware(void)
{
    uint8_t *appPtr = 0;
    flashReadGetPointer(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR, &appPtr);
    flashDeinit(FLASH_DEVICE_DEFAULT);
    uint32_t appAddr = (uint32_t) appPtr;
    uint32_t* RHAddr = (uint32_t*)*((uint32_t*)((uint32_t)appAddr + 4));
    void (*jumpToApp)(void) = (void (*)(void)) RHAddr;
    // set vector table, set stack pointer and boot
    __set_MSP(*(uint32_t*)appAddr);
    SCB->VTOR = (uint32_t)appAddr;
    //bye!
    jumpToApp();
    while(1);
}

int main(void)
{
    dprint("bootloader pyco");
    flashInit(FLASH_DEVICE_DEFAULT);
    if (((__bl_act & BL_ACT_DIRECTION_MASK) != BL_ACT_APPTOBL) ||
            ((__bl_act & BL_ACT_ACTION_MASK) == 0)) {
        // we can try to boot immediately
        if (flashCheckAppImage(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR, 0) == FLUPDATE_OK) {
            // firmware is OK (CRC match)
            dprint("firmware ok");
            bootFirmware();
        }
    }
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
