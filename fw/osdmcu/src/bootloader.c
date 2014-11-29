/*
    DRACO - Copyright (C) 2014 Daniel Strnad

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
#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "led.h"
#include "swtimer.h"
#include "delay.h"
#include "bootloader.h"
#include "flashupdate.h"
#include "spicomm.h"
#include "spicommio_stm32f3.h"
#include "comm_commands.h"
#include "debug.h"
#include "version.h"
#include "utils.h"

void HSyncInterrupt() {}
void CSyncInterrupt() {}
void VSyncInterrupt() {}

static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen);
static const SpiCommConfig spiCommConfig = {
        .cbPrivData = 0,
        .io = &spiCommIo,
        .onRequestReceived = onRequestReceived,
};
static SpiComm spiComm;
static bool bootPlanned;
static bool flashingPerformed;
static uint32_t bootTimeMs;

static void onRequestReceived(void *priv, uint8_t *data, uint8_t len, uint8_t *ansData, uint8_t *ansLen)
{
    if (!len) {
        *ansLen = 0;
        return;
    }

    switch (data[0]) {
    case REQ_ID_VERSION: {
        ansData[0] = COMM_VERSION_MODE_BOOTLOADER;
        strcpy((char*)(&ansData[1]), __firmwareVersion);
        *ansLen = strlen(__firmwareVersion) + 1;
    }
        break;

    case REQ_ID_WRITE_START: {
        uint32_t size = GETLE32(&data[1]);
        uint32_t offset = GETLE32(&data[5]);
        if (size > FLASH_APPLICATION_MAX_SIZE || len < 9) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        if (flashStartSequentialWrite(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR + offset) != FLUPDATE_OK) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1;
    }
    break;

    case REQ_ID_WRITE_CHUNK: {
        if (flashSequentialWrite(FLASH_DEVICE_DEFAULT, &data[1], len - 1) != FLUPDATE_OK) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1;
    }
    break;

    case REQ_ID_READ_START: {
        if (len < 5) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        uint32_t offset = GETLE32(&data[1]);
        if (flashStartSequentialRead(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR + offset) != FLUPDATE_OK) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        flashingPerformed = true;
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1;
    }
    break;

    case REQ_ID_READ_CHUNK:{
        if (len < 2) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        uint8_t blockSize = data[1];
        if (flashSequentialRead(FLASH_DEVICE_DEFAULT, &ansData[1], blockSize) != FLUPDATE_OK) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1 + blockSize;
    }
    break;

    case REQ_ID_EXIT_BOOT: {
        if (flashCheckAppImage(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR, 0) != FLUPDATE_OK) {
            ansData[0] = COMM_RESULT_ERROR;
            *ansLen = 1;
            return;
        }
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1;
        bootPlanned = true;
        bootTimeMs = getElapsedMs();
    }
    break;

    case REQ_ID_ENTER_BOOT: {
        // we are already in boot mode
        ansData[0] = COMM_RESULT_OK;
        *ansLen = 1;
    }
    break;

    default: {
        *ansLen = 0;
    }
    break;
    }
}

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
    flashInit(FLASH_DEVICE_DEFAULT);
    if (((__bl_act & BL_ACT_DIRECTION_MASK) != BL_ACT_APPTOBL) ||
            ((__bl_act & BL_ACT_ACTION_MASK) == 0)) {
        // we can try to boot immediately
        if (flashCheckAppImage(FLASH_DEVICE_DEFAULT, FLASH_APPLICATION_MAIN_ADDR, 0) == FLUPDATE_OK) {
            // firmware is OK (CRC match)
            bootFirmware();
        }
    }

    // image was corrupted or bootloader mode was requested
    boardInit();

    // make sure video signal can passthrough while
    // we are in bootloader
    gpioChangeMode(PinOsdMask, GPIO_Mode_OUT);
    gpioControl(PinOsdMask, 1);

    sysTimerInit();
    ledInit();
    spiCommStart(&spiComm, &spiCommConfig);
    while(1) {
        spiCommProcess(&spiComm);
        if (bootPlanned) {
            if ((getElapsedMs() - bootTimeMs) > 20) {
                __bl_act = BL_ACT_BLTOAPP | BL_ACT_BL_ACTIVITY;
                if (flashingPerformed)
                    __bl_act |= BL_ACT_BL_FLASH;
                NVIC_SystemReset();
                while(1);
            }
        }
    }

    return 0;
}
