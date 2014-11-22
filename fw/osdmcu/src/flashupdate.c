/*
 * flashupdate.c
 *
 *  Created on: 6. 11. 2014
 *      Author: dan
 */

#include <stdbool.h>
#include "flashupdate.h"
#include "debug.h"

#define APP_SIZE_ADDR(APPOFFSET)        ((uint32_t)(APPOFFSET) + 0x200)
#define APP_CRC_ADDR(APPOFFSET)         ((uint32_t)(APPOFFSET) + *(uint32_t*)APP_SIZE_ADDR((uint32_t)(APPOFFSET)) - 0x04)
#define APP_TIMESTAMP_ADDR(APPOFFSET)   (APP_CRC_ADDR((uint32_t)(APPOFFSET)) - 0x04)
#define APP_VERSION_ADDR(APPOFFSET)     (APP_TIMESTAMP_ADDR((uint32_t)(APPOFFSET)) - 0x20)

#define FLASH_SR_CLEARERR               FLASH_SR_WRPERR | FLASH_SR_EOP | FLASH_SR_PGERR

struct FlashPage {
    uint32_t offset;
    uint32_t size;
    uint32_t eraseTime;
    uint32_t wordProgramTime;
};

struct FlashDevice {
    uint32_t writeAddr;
    bool seqWriteInProgress;
    bool seqWritePageErase;
    const struct FlashDeviceLayout *layout;
};

struct FlashDeviceLayout {
    uint32_t pageCount;
    uint32_t base;
    uint32_t size;
    const struct FlashPage *pages;
};

const struct FlashPage stm32f303ccFlashPages[128] = {
        {0x00000000, 2048, 40000, 60},
        {0x00000800, 2048, 40000, 60},
        {0x00001000, 2048, 40000, 60},
        {0x00001800, 2048, 40000, 60},
        {0x00002000, 2048, 40000, 60},
        {0x00002800, 2048, 40000, 60},
        {0x00003000, 2048, 40000, 60},
        {0x00003800, 2048, 40000, 60},
        {0x00004000, 2048, 40000, 60},
        {0x00004800, 2048, 40000, 60},
        {0x00005000, 2048, 40000, 60},
        {0x00005800, 2048, 40000, 60},
        {0x00006000, 2048, 40000, 60},
        {0x00006800, 2048, 40000, 60},
        {0x00007000, 2048, 40000, 60},
        {0x00007800, 2048, 40000, 60},
        {0x00008000, 2048, 40000, 60},
        {0x00008800, 2048, 40000, 60},
        {0x00009000, 2048, 40000, 60},
        {0x00009800, 2048, 40000, 60},
        {0x0000a000, 2048, 40000, 60},
        {0x0000a800, 2048, 40000, 60},
        {0x0000b000, 2048, 40000, 60},
        {0x0000b800, 2048, 40000, 60},
        {0x0000c000, 2048, 40000, 60},
        {0x0000c800, 2048, 40000, 60},
        {0x0000d000, 2048, 40000, 60},
        {0x0000d800, 2048, 40000, 60},
        {0x0000e000, 2048, 40000, 60},
        {0x0000e800, 2048, 40000, 60},
        {0x0000f000, 2048, 40000, 60},
        {0x0000f800, 2048, 40000, 60},
        {0x00010000, 2048, 40000, 60},
        {0x00010800, 2048, 40000, 60},
        {0x00011000, 2048, 40000, 60},
        {0x00011800, 2048, 40000, 60},
        {0x00012000, 2048, 40000, 60},
        {0x00012800, 2048, 40000, 60},
        {0x00013000, 2048, 40000, 60},
        {0x00013800, 2048, 40000, 60},
        {0x00014000, 2048, 40000, 60},
        {0x00014800, 2048, 40000, 60},
        {0x00015000, 2048, 40000, 60},
        {0x00015800, 2048, 40000, 60},
        {0x00016000, 2048, 40000, 60},
        {0x00016800, 2048, 40000, 60},
        {0x00017000, 2048, 40000, 60},
        {0x00017800, 2048, 40000, 60},
        {0x00018000, 2048, 40000, 60},
        {0x00018800, 2048, 40000, 60},
        {0x00019000, 2048, 40000, 60},
        {0x00019800, 2048, 40000, 60},
        {0x0001a000, 2048, 40000, 60},
        {0x0001a800, 2048, 40000, 60},
        {0x0001b000, 2048, 40000, 60},
        {0x0001b800, 2048, 40000, 60},
        {0x0001c000, 2048, 40000, 60},
        {0x0001c800, 2048, 40000, 60},
        {0x0001d000, 2048, 40000, 60},
        {0x0001d800, 2048, 40000, 60},
        {0x0001e000, 2048, 40000, 60},
        {0x0001e800, 2048, 40000, 60},
        {0x0001f000, 2048, 40000, 60},
        {0x0001f800, 2048, 40000, 60},
        {0x00020000, 2048, 40000, 60},
        {0x00020800, 2048, 40000, 60},
        {0x00021000, 2048, 40000, 60},
        {0x00021800, 2048, 40000, 60},
        {0x00022000, 2048, 40000, 60},
        {0x00022800, 2048, 40000, 60},
        {0x00023000, 2048, 40000, 60},
        {0x00023800, 2048, 40000, 60},
        {0x00024000, 2048, 40000, 60},
        {0x00024800, 2048, 40000, 60},
        {0x00025000, 2048, 40000, 60},
        {0x00025800, 2048, 40000, 60},
        {0x00026000, 2048, 40000, 60},
        {0x00026800, 2048, 40000, 60},
        {0x00027000, 2048, 40000, 60},
        {0x00027800, 2048, 40000, 60},
        {0x00028000, 2048, 40000, 60},
        {0x00028800, 2048, 40000, 60},
        {0x00029000, 2048, 40000, 60},
        {0x00029800, 2048, 40000, 60},
        {0x0002a000, 2048, 40000, 60},
        {0x0002a800, 2048, 40000, 60},
        {0x0002b000, 2048, 40000, 60},
        {0x0002b800, 2048, 40000, 60},
        {0x0002c000, 2048, 40000, 60},
        {0x0002c800, 2048, 40000, 60},
        {0x0002d000, 2048, 40000, 60},
        {0x0002d800, 2048, 40000, 60},
        {0x0002e000, 2048, 40000, 60},
        {0x0002e800, 2048, 40000, 60},
        {0x0002f000, 2048, 40000, 60},
        {0x0002f800, 2048, 40000, 60},
        {0x00030000, 2048, 40000, 60},
        {0x00030800, 2048, 40000, 60},
        {0x00031000, 2048, 40000, 60},
        {0x00031800, 2048, 40000, 60},
        {0x00032000, 2048, 40000, 60},
        {0x00032800, 2048, 40000, 60},
        {0x00033000, 2048, 40000, 60},
        {0x00033800, 2048, 40000, 60},
        {0x00034000, 2048, 40000, 60},
        {0x00034800, 2048, 40000, 60},
        {0x00035000, 2048, 40000, 60},
        {0x00035800, 2048, 40000, 60},
        {0x00036000, 2048, 40000, 60},
        {0x00036800, 2048, 40000, 60},
        {0x00037000, 2048, 40000, 60},
        {0x00037800, 2048, 40000, 60},
        {0x00038000, 2048, 40000, 60},
        {0x00038800, 2048, 40000, 60},
        {0x00039000, 2048, 40000, 60},
        {0x00039800, 2048, 40000, 60},
        {0x0003a000, 2048, 40000, 60},
        {0x0003a800, 2048, 40000, 60},
        {0x0003b000, 2048, 40000, 60},
        {0x0003b800, 2048, 40000, 60},
        {0x0003c000, 2048, 40000, 60},
        {0x0003c800, 2048, 40000, 60},
        {0x0003d000, 2048, 40000, 60},
        {0x0003d800, 2048, 40000, 60},
        {0x0003e000, 2048, 40000, 60},
        {0x0003e800, 2048, 40000, 60},
        {0x0003f000, 2048, 40000, 60},
        {0x0003f800, 2048, 40000, 60},
};

const struct FlashDeviceLayout stm32f303ccFlashLayout = {
        .base = 0x08000000,
        .pageCount = sizeof(stm32f303ccFlashPages) / sizeof(struct FlashPage),
        .size = 0x80000,
        .pages = &stm32f303ccFlashPages[0],
};

struct FlashDevice stm32f303ccFlash = {
        .layout = &stm32f303ccFlashLayout,
};
FlashDevice_t stm32f303ccFlashDevice = &stm32f303ccFlash;



static uint32_t crc32(const uint32_t oldCRC, const uint8_t new_byte)
{
    uint32_t  crc;
    uint8_t i;

    crc = oldCRC ^ new_byte;
    i = 8;
    do {
        if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320L; // CRC-32 reverse polynom
        else crc = crc >> 1;
    } while(--i);
    return (crc);
}

static uint32_t calcCRC32(uint8_t *block, const uint32_t count)
{
  uint32_t crc=0xffffffff;
  uint32_t cnt=count;
  uint8_t *uk;
  uk=block;
  while(cnt--) {
      crc=crc32(crc,*uk++);
  }
  crc ^= 0xffffffff;
  return(crc);
}

void flashInit(FlashDevice_t dev)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    flash->writeAddr = 0;
    flash->seqWriteInProgress = false;
    flash->seqWritePageErase = 0;

    FLASH->CR = FLASH_CR_LOCK;
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

void flashDeinit(FlashDevice_t dev)
{
    (void) dev;
    FLASH->CR |= FLASH_CR_LOCK;
}

static enum FlashUpdateRes flashWaitDone(FlashDevice_t dev)
{
    (void) dev;
    while (FLASH->SR & FLASH_SR_BSY);
    if ((FLASH->SR & FLASH_SR_EOP) == 0) {
        FLASH->SR = FLASH_SR_CLEARERR;
        return FLUPDATE_ERR_PROGRAMMING;
    }

    if (FLASH->SR & FLASH_SR_PGERR) {
        FLASH->SR = FLASH_SR_CLEARERR;
        return FLUPDATE_ERR_PROGRAMMING;
    }

    if (FLASH->SR & FLASH_SR_WRPERR) {
        FLASH->SR = FLASH_SR_CLEARERR;
        return FLUPDATE_ERR_PROGRAMMING;
    }

    FLASH->SR = FLASH_SR_CLEARERR;
    return FLUPDATE_OK;
}

static enum FlashUpdateRes flashErasePage(FlashDevice_t dev, uint8_t pageNr)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    (void) flash;
    FLASH->CR = FLASH_CR_PER;
    FLASH->AR = flash->layout->base + flash->layout->pages[pageNr].offset;
    FLASH->CR |= FLASH_CR_STRT;
    return flashWaitDone(dev);
}

static enum FlashUpdateRes flashProgramWord(FlashDevice_t dev, uint32_t addr, uint16_t word)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    (void) flash;
    FLASH->CR = FLASH_CR_PG;
    *(volatile uint16_t*)(addr + flash->layout->base) = word;

    enum FlashUpdateRes res = flashWaitDone(dev);
    FLASH->CR &= ~FLASH_CR_PG;
    if (res != FLUPDATE_OK)
        return FLUPDATE_ERR_PROGRAMMING;

    if (*(volatile uint16_t*)(addr + flash->layout->base) != word)
        return FLUPDATE_ERR_PROGRAMMING;

    return FLUPDATE_OK;
}

static uint8_t flashAddrToPage(FlashDevice_t dev, uint32_t addr)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    uint8_t pageNr = 0;
    uint32_t page;
    for (page = 0; page < flash->layout->pageCount; page++) {
        if (flash->layout->pages[page].offset > addr)
            break;
        pageNr++;
    }
    if (pageNr > 0) pageNr--;
    return pageNr;
}

enum FlashUpdateRes flashStartSequentialWrite(FlashDevice_t dev, uint32_t addr)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    if (addr > (flash->layout->size - 1))
        return FLUPDATE_ERR_ADDR_OUT_OF_RANGE;
    flash->seqWriteInProgress = true;
    flash->writeAddr = addr;
    flash->seqWritePageErase = true;

    return FLUPDATE_OK;
}

enum FlashUpdateRes flashEndSequentialWrite(FlashDevice_t dev)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    flash->seqWriteInProgress = false;
    return FLUPDATE_OK;
}

enum FlashUpdateRes flashSequentialWrite(FlashDevice_t dev, uint8_t *data, uint32_t size)
{

    struct FlashDevice *flash = (struct FlashDevice*)dev;
    enum FlashUpdateRes res;
    if (!flash->seqWriteInProgress)
        return FLUPDATE_ERR_NOT_READY;
    if ((size + flash->writeAddr) > (flash->layout->size))
        return FLUPDATE_ERR_ADDR_OUT_OF_RANGE;
    if (size == 0)
        return FLUPDATE_OK;

    // erase what we need
    uint8_t page = 0, pageLast = 0;
    bool firstErase = true;
    for (page = 0; page < flash->layout->pageCount; page++) {
        uint32_t a = flash->writeAddr;
        uint32_t b = flash->writeAddr + (size - 1);
        uint32_t c = flash->layout->pages[page].offset;
        uint32_t d = flash->layout->pages[page].offset + (flash->layout->pages[page].size - 1);
        if (((a >= c) && (a <= d)) || ((b >= c) && (b <= d)) || ((a < c) && (b > d))) {
            pageLast = page;
            bool erase = true;
            if (firstErase) {
                firstErase = false;
                if (!flash->seqWritePageErase)
                    erase = false;
            }
            if (erase) {
                res = flashErasePage(dev, page);
                if (res != FLUPDATE_OK) {
                    flash->seqWriteInProgress = false;
                    return res;
                }
            }
        }
    }

    uint32_t b = 0;
    uint16_t word = 0;
    for (b = 0; b < size; b++) {
        bool prog = false;
        switch(b & 0x01) {
        case 0: word = data[b]; break;
        case 1: word |= data[b] << 8; prog = true; break;
        }
        if (b == (size - 1)) {
            prog = true;
        }

        if (prog) {
            res = flashProgramWord(dev, (flash->writeAddr + b) & 0xfffffffe, word);
            if (res != FLUPDATE_OK) {
                flash->seqWriteInProgress = false;
                return res;
            }
        }
    }
    flash->writeAddr += size;
    if (pageLast != flashAddrToPage(dev, flash->writeAddr)) {
        flash->seqWritePageErase = true;
    } else {
        flash->seqWritePageErase = false;
    }

    return FLUPDATE_OK;
}

uint32_t flashTimeSequentialWrite(FlashDevice_t dev, uint32_t size)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    if (!flash->seqWriteInProgress)
        return 0;
    if ((size + flash->writeAddr) > (flash->layout->size))
        return 0;
    if (size == 0)
        return 0;

    uint32_t time = ((size / 2) + 1) * flash->layout->pages[0].wordProgramTime;
    uint8_t page = 0;
    bool firstErase = true;
    for (page = 0; page < flash->layout->pageCount; page++) {
        uint32_t a = flash->writeAddr;
        uint32_t b = flash->writeAddr + (size - 1);
        uint32_t c = flash->layout->pages[page].offset;
        uint32_t d = flash->layout->pages[page].offset + (flash->layout->pages[page].size - 1);
        if (((a >= c) && (a <= d)) || ((b >= c) && (b <= d)) || ((a < c) && (b > d))) {
            if (firstErase) {
                firstErase = false;
                if (!flash->seqWritePageErase)
                    continue;
            }
            time += flash->layout->pages[page].eraseTime;
        }
    }
    return time / 1000;
}

enum FlashUpdateRes flashEraseArea(FlashDevice_t dev, uint32_t address, uint32_t size)
{
    struct FlashDevice *flash = (struct FlashDevice*)dev;
    // erase what we need
    uint8_t page = 0;
    for (page = 0; page < flash->layout->pageCount; page++) {
        uint32_t a = address;
        uint32_t b = address + (size - 1);
        uint32_t c = flash->layout->pages[page].offset;
        uint32_t d = flash->layout->pages[page].offset + (flash->layout->pages[page].size - 1);
        if (((a >= c) && (a <= d)) || ((b >= c) && (b <= d)) || ((a < c) && (b > d))) {
            enum FlashUpdateRes res = flashErasePage(dev, page);
            if (res != FLUPDATE_OK) {
                return res;
            }
        }
    }

    return FLUPDATE_OK;
}

enum FlashUpdateRes flashCheckAppImage(FlashDevice_t dev, uint32_t appAddr, uint32_t *imageSize)
{
    const struct FlashDevice *flash = (const struct FlashDevice*)dev;
    uint32_t crc;
    if (((uint32_t)appAddr+512+4+32+4+4) >(flash->layout->size)) {
        return FLUPDATE_ERR_IMAGE_CHECK_FAILED_SIZE;
    }

    uint32_t appSize = *(uint32_t*)APP_SIZE_ADDR(appAddr + flash->layout->base);
    if (imageSize != 0)
        *imageSize = appSize;
    if (appSize > (flash->layout->size + ((uint32_t)appAddr))) {
        return FLUPDATE_ERR_IMAGE_CHECK_FAILED_SIZE;
    }
    if ((appSize<40) || (appSize > FLASH_APPLICATION_MAX_SIZE)) {
        return FLUPDATE_ERR_IMAGE_CHECK_FAILED_SIZE;
    }
    uint32_t appCrc = *(uint32_t*)APP_CRC_ADDR(appAddr + flash->layout->base);
    crc = calcCRC32((uint8_t*)(appAddr + flash->layout->base), appSize - 4);
    if (crc != appCrc) {
        return FLUPDATE_ERR_IMAGE_CHECK_FAILED_CRC;
    } else {
        return FLUPDATE_OK;
    }
}

enum FlashUpdateRes flashReadArea(FlashDevice_t dev, uint8_t *dst, uint32_t address, uint32_t offset, uint32_t count)
{
    const struct FlashDevice *flash = (const struct FlashDevice*)dev;

    if (address + offset + count > flash->layout->size)
        return FLUPDATE_ERR_ADDR_OUT_OF_RANGE;

    uint32_t i = 0;
    for (i = 0; i < count; i++) {
        uint8_t *addr = (uint8_t * )(flash->layout->base + address + offset + i);
        dst[i] = *addr;
    }

    return FLUPDATE_OK;
}

enum FlashUpdateRes flashReadGetPointer(FlashDevice_t dev, uint32_t address, uint8_t **ptr)
{
    const struct FlashDevice *flash = (const struct FlashDevice*)dev;
    *ptr = (uint8_t *)(flash->layout->base + address);

    return FLUPDATE_OK;
}
