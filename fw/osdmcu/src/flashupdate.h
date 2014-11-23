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
 * @file    flashupdate.h
 * @brief   flash memory manipulation
 *
 */

#include <stdint.h>

#ifndef SRC_FLASHUPDATE_H_
#define SRC_FLASHUPDATE_H_
#define FLASH_APPLICATION_MAX_SIZE              245760
#define FLASH_APPLICATION_MAIN_ADDR         0x00004000

#define FLASH_DEVICE_DEFAULT                stm32f303ccFlashDevice
enum FlashUpdateRes {
    FLUPDATE_OK,
    FLUPDATE_ERR_IMAGE_CHECK_FAILED_SIZE,
    FLUPDATE_ERR_IMAGE_CHECK_FAILED_CRC,
    FLUPDATE_ERR_ADDR_OUT_OF_RANGE,
    FLUPDATE_ERR_NOT_READY,
    FLUPDATE_ERR_PROGRAMMING,
};

typedef void* FlashDevice_t;

extern FlashDevice_t stm32f303ccFlashDevice;

void flashInit(FlashDevice_t dev);
void flashDeinit(FlashDevice_t dev);
enum FlashUpdateRes flashReadArea(FlashDevice_t dev, uint8_t *dst, uint32_t address, uint32_t offset, uint32_t count);
enum FlashUpdateRes flashReadGetPointer(FlashDevice_t dev, uint32_t address, uint8_t **ptr);
enum FlashUpdateRes flashCheckAppImage(FlashDevice_t dev, uint32_t appAddr, uint32_t *imageSize);
enum FlashUpdateRes flashStartSequentialWrite(FlashDevice_t dev, uint32_t addr);
enum FlashUpdateRes flashEndSequentialWrite(FlashDevice_t dev);
enum FlashUpdateRes flashSequentialWrite(FlashDevice_t dev, uint8_t *data, uint32_t size);
uint32_t flashTimeSequentialWrite(FlashDevice_t dev, uint32_t size);
enum FlashUpdateRes flashEraseArea(FlashDevice_t dev, uint32_t address, uint32_t size);
enum FlashUpdateRes flashStartSequentialRead(FlashDevice_t dev, uint32_t addr);
enum FlashUpdateRes flashSequentialRead(FlashDevice_t dev, uint8_t *data, uint32_t size);

#endif /* SRC_FLASHUPDATE_H_ */
