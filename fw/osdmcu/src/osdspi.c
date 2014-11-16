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
 * @file    osd.c
 * @brief   STM32F3 SPI painting device
 *
 */

#include "osdspi.h"
#include "board.h"
#include "delay.h"
#include "debug.h"
#include "utils.h"
#include <string.h>
#include <stm32f30x.h>
#include <stm32f30x_spi.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_dma.h>
#include <stm32f30x_tim.h>

#define VIDEO_BUFFER_HRES       320
#define VIDEO_BUFFER_VRES       240
#define LINE_BUFFER_PADDING     12
#define VIDEO_BUFFER_SIZE       ((VIDEO_BUFFER_HRES * VIDEO_BUFFER_VRES) / 32)

#define STATE_NOT_INITIALIZED           0
#define STATE_SYSTEM_DETECTION          1
#define STATE_RUNNING                   2
#define DETECTION_PAL_LINE_THRESHOLD    280

VIDEO_BUFFER_MEM static uint32_t levelBuffer[2][VIDEO_BUFFER_SIZE];
VIDEO_BUFFER_MEM static uint32_t maskBuffer[2][VIDEO_BUFFER_SIZE];
VIDEO_BUFFER_MEM static uint32_t levelLineBuffer[2][VIDEO_BUFFER_HRES / 32 + LINE_BUFFER_PADDING / 4];
VIDEO_BUFFER_MEM static uint32_t maskLineBuffer[2][VIDEO_BUFFER_HRES / 32 + 4 + LINE_BUFFER_PADDING / 4];

static uint32_t *levelLine;
static uint32_t *maskLine;

volatile uint32_t dbgStart = 0, dbgStop = 0;

typedef struct {
    const char *name;
    uint16_t vOffsetLines;
    uint8_t offsetBytes;
    uint16_t lineCount;
    uint32_t linePeriod;
}TvSystem;

typedef struct {
    volatile uint8_t state;
    volatile uint32_t linecounter;
    volatile uint8_t lineInProgress;
    const TvSystem *tvSys;
    uint32_t pxClk;
    uint32_t pxPeriod;
    uint16_t hres;
    uint16_t vres;
    uint32_t *fLevelBuffer;
    uint32_t *fMaskBuffer;
    uint32_t *bLevelBuffer;
    uint32_t *bMaskBuffer;
    uint8_t swapRequest;
    uint8_t waitNextField;
    uint8_t tvsysForced;
    BufferSwappedCb bufferSwappedCb;
    void *cbctx;
}Osd;

const TvSystem tvPal = {
        .name = "PAL",
        .vOffsetLines = 44,
        .offsetBytes = 5,
        .lineCount = 240,
        .linePeriod = 54000,
};

const TvSystem tvNtsc = {
        .name = "NTSC",
        .vOffsetLines = 16,
        .offsetBytes = 5,
        .lineCount = 240,
        .linePeriod = 54000,
};

static Osd osd;

static void switchTvSystem(const TvSystem *tvSys);

static void prepareLine(uint16_t num)
{

    if (levelLine != &levelLineBuffer[0][0]) levelLine = &levelLineBuffer[0][0];
    else levelLine = &levelLineBuffer[1][0];

    if (maskLine != &maskLineBuffer[0][0]) maskLine = &maskLineBuffer[0][0];
    else maskLine = &maskLineBuffer[1][0];

    int i;
    memset(maskLine, 0xff, sizeof(levelLineBuffer) / 2);

    uint32_t row = (VIDEO_BUFFER_HRES / 32) * num;

    // warning: this may lead to unaligned memory access (ok on CM3 and CM4)
    uint32_t *level = (uint32_t *)&((uint8_t *)(levelLine))[osd.tvSys->offsetBytes];
    uint32_t *mask = (uint32_t *)&((uint8_t *)(maskLine))[osd.tvSys->offsetBytes];

    for (i = 0; i < (VIDEO_BUFFER_HRES / 32); i++) {
        uint32_t word = osd.fLevelBuffer[row + i];
        level[i] = word; /*__REV(word);*/
        word = osd.fMaskBuffer[row + i];
        mask[i] = word^0xffffffff;/*__REV(word^0xffffffff);*/
    }
}

static void prepareLinePeriphs(void)
{
    SPI_LEVEL_DMA_CH_TX->CMAR = (uint32_t)levelLine;
    SPI_MASK_DMA_CH_TX->CMAR = (uint32_t)maskLine;
    SPI_LEVEL_DMA_CH_TX->CNDTR = osd.hres / 8 + osd.tvSys->offsetBytes / 1;
    SPI_MASK_DMA_CH_TX->CNDTR = osd.hres / 8 + osd.tvSys->offsetBytes / 1;
    SPI_MASK->CR1 &= SPI_NSSInternalSoft_Reset;
    // SPI_Cmd(SPI_MASK, ENABLE);
    SPI_MASK_DMA_CH_TX->CCR |= DMA_CCR_EN;
    SPI_LEVEL_DMA_CH_TX->CCR |= DMA_CCR_EN;
}

void CSyncInterrupt(void)
{
}

void HSyncInterrupt(void)
{
    if (osd.state == STATE_NOT_INITIALIZED) return;
    if ((osd.state == STATE_RUNNING) && (!osd.waitNextField) && (!osd.lineInProgress)) {
        if ((osd.linecounter >= osd.tvSys->vOffsetLines) && ((osd.linecounter - osd.tvSys->vOffsetLines) < osd.vres)) {
            osd.lineInProgress = 1;
            SPI_MASK->CR1 |= SPI_CR1_SPE;
            SPI_LEVEL->CR1 |= SPI_CR1_SPE;

            if (((osd.linecounter + 1) >= osd.tvSys->vOffsetLines) &&
                    (((osd.linecounter + 1) - osd.tvSys->vOffsetLines) < osd.vres))
                prepareLine(osd.linecounter + 1 - osd.tvSys->vOffsetLines);
        }
    }
    osd.linecounter++;
}

static inline void swapVideoBuffers(void)
{
    uint32_t *tmp = 0;
    tmp = osd.bLevelBuffer;
    osd.bLevelBuffer = osd.fLevelBuffer;
    osd.fLevelBuffer = tmp;
    tmp = osd.bMaskBuffer;
    osd.bMaskBuffer = osd.fMaskBuffer;
    osd.fMaskBuffer = tmp;
}

void VSyncInterrupt(void)
{
    if (osd.state == STATE_NOT_INITIALIZED) {
        return;
    } else if (osd.state == STATE_SYSTEM_DETECTION) {
        if (osd.waitNextField) {
            osd.waitNextField = 0;
        } else {
            if (osd.linecounter > DETECTION_PAL_LINE_THRESHOLD) {
                switchTvSystem(&tvPal);
            } else {
                switchTvSystem(&tvNtsc);
            }
            osd.state = STATE_RUNNING;
        }
        osd.linecounter = 0;
    }

    if (osd.state == STATE_RUNNING) {
        osd.linecounter = 0;
        osd.waitNextField = 0;
        if (osd.swapRequest) {
            swapVideoBuffers();
            osd.swapRequest = 0;
            if (osd.bufferSwappedCb) osd.bufferSwappedCb(osd.cbctx, osd.bLevelBuffer, osd.bMaskBuffer);
        }
        prepareLine(0);
    }
}

void SPI_LEVEL_DMA_CH_IRQH(void)
{
    SPI_LEVEL_DMA->IFCR = 1 << ((SPI_LEVEL_DMA_CH_TX_N - 1) * 4);
    DMA_Cmd(SPI_LEVEL_DMA_CH_TX, DISABLE);
    // send FF to mask to left last bit on MISO in log. 1 state (transparent)
    while(SPI_I2S_GetFlagStatus(SPI_MASK, SPI_I2S_FLAG_TXE) == 0);
    SPI_SendData8(SPI_MASK, 0xff);
    while(SPI_I2S_GetFlagStatus(SPI_LEVEL, SPI_I2S_FLAG_TXE) == 0);
    SPI_SendData8(SPI_LEVEL, 0xff);

    while (SPI_GetTransmissionFIFOStatus(SPI_LEVEL) != SPI_TransmissionFIFOStatus_Empty);
    while (SPI_I2S_GetFlagStatus(SPI_LEVEL, SPI_I2S_FLAG_BSY));
    while (SPI_I2S_GetFlagStatus(SPI_MASK, SPI_I2S_FLAG_BSY));

    SPI_Cmd(SPI_LEVEL, DISABLE);
    SPI_Cmd(SPI_MASK, DISABLE);

    prepareLinePeriphs();
    osd.lineInProgress = 0;
}

void SPI_MASK_DMA_CH_IRQH(void)
{
    SPI_MASK_DMA->IFCR = 1 << ((SPI_MASK_DMA_CH_TX_N - 1) * 4);
    DMA_Cmd(SPI_MASK_DMA_CH_TX, DISABLE);
}

static void switchTvSystem(const TvSystem *tvSys)
{
    disableIrqs();
    osd.tvSys = tvSys;
    osd.vres = (VIDEO_BUFFER_VRES < osd.tvSys->lineCount) ? VIDEO_BUFFER_VRES : osd.tvSys->lineCount;
    uint32_t pxcnt = (osd.tvSys->linePeriod / osd.pxPeriod);
    osd.hres = (VIDEO_BUFFER_HRES < pxcnt) ? VIDEO_BUFFER_HRES : pxcnt;

    RCC_ClocksTypeDef rccClocks;
    RCC_GetClocksFreq(&rccClocks);

    enableIrqs();
}

void osdSpiChangeTvSystem(enum OsdSpiTvSystem tvsys)
{
    switch (tvsys) {
    case OSDSPI_TVSYSTEM_PAL:
        if (osd.state != STATE_NOT_INITIALIZED)
            osd.state = STATE_RUNNING;

        switchTvSystem(&tvPal);
        break;
    case OSDSPI_TVSYSTEM_NTSC:
        if (osd.state != STATE_NOT_INITIALIZED)
            osd.state = STATE_RUNNING;

        switchTvSystem(&tvNtsc);
        break;
    default:

        break;
    }
}

static uint32_t getSpiClk(SPI_TypeDef *spi)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    uint32_t pclk;
    switch((uint32_t)spi) {
    case (uint32_t)SPI1:
        pclk =  clocks.PCLK2_Frequency;
        break;

    case (uint32_t)SPI2:
    case (uint32_t)SPI3:
        pclk = clocks.PCLK1_Frequency;
        break;

    default:
        pclk = 0;
        break;
    }

    uint32_t prescaler = 1 << (((spi->CR1 >> 3) & 0x07) + 1);
    return pclk / prescaler;
}

static void deviceStop(void *priv)
{
    (void) priv;
    osd.state = STATE_NOT_INITIALIZED;
    DMA_Cmd(SPI_MASK_DMA_CH_TX, DISABLE);
    DMA_Cmd(SPI_LEVEL_DMA_CH_TX, DISABLE);
    SPI_MASK_RCC_RESET;
    SPI_LEVEL_RCC_RESET;
}

static int deviceStart(void *priv)
{
    (void) priv;
    osd.state = STATE_NOT_INITIALIZED;

    SPI_LEVEL_RCC_ENABLE;
    SPI_MASK_RCC_ENABLE;
    SPI_LEVEL_RCC_RESET;
    SPI_MASK_RCC_RESET;

    SPI_InitTypeDef spiLevelConf = {
            .SPI_Direction = SPI_Direction_1Line_Tx,
            .SPI_Mode = SPI_Mode_Master,
            .SPI_DataSize = SPI_DataSize_8b,
            .SPI_CPOL = SPI_CPOL_High,
            .SPI_CPHA = SPI_CPHA_2Edge,
            .SPI_NSS  = SPI_NSS_Soft,
            .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4, // 7MHz pixel clock
            .SPI_FirstBit = SPI_FirstBit_LSB,
    };
    SPI_Init(SPI_LEVEL, &spiLevelConf);
    SPI_I2S_DMACmd(SPI_LEVEL, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_ITConfig(SPI_MASK, SPI_I2S_IT_TXE, ENABLE);
    SPI_LastDMATransferCmd(SPI_LEVEL, SPI_LastDMATransfer_TxEvenRxEven);

    SPI_InitTypeDef spiMaskConf = {
            .SPI_Direction = SPI_Direction_1Line_Tx,
            .SPI_Mode = SPI_Mode_Slave,
            .SPI_DataSize = SPI_DataSize_8b,
            .SPI_CPOL = SPI_CPOL_High,
            .SPI_CPHA = SPI_CPHA_2Edge,
            .SPI_NSS  = SPI_NSS_Soft,
            .SPI_BaudRatePrescaler = 0, // clocked by SPI_LEVEL
            .SPI_FirstBit = SPI_FirstBit_LSB,
    };
    SPI_Init(SPI_MASK, &spiMaskConf);
    SPI_I2S_DMACmd(SPI_MASK, SPI_I2S_DMAReq_Tx, ENABLE);

    // TODO should be this interrupt handled??
    SPI_I2S_ITConfig(SPI_MASK, SPI_I2S_IT_TXE, ENABLE);
    SPI_LastDMATransferCmd(SPI_MASK, SPI_LastDMATransfer_TxEvenRxEven);

    DMA_InitTypeDef dmaLevelConf = {
            .DMA_PeripheralBaseAddr = (uint32_t)&SPI_LEVEL->DR,
            .DMA_MemoryBaseAddr = 0,
            .DMA_DIR = DMA_DIR_PeripheralDST,
            .DMA_BufferSize = 0,
            .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
            .DMA_MemoryInc = DMA_MemoryInc_Enable,
            .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
            .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
            .DMA_Mode = DMA_Mode_Normal,
            .DMA_Priority = DMA_Priority_Medium,
            .DMA_M2M = DMA_M2M_Disable,
    };
    DMA_Init(SPI_LEVEL_DMA_CH_TX, &dmaLevelConf);
    DMA_ITConfig(SPI_LEVEL_DMA_CH_TX, DMA_IT_TC, ENABLE);
    DMA_InitTypeDef dmaMaskConf = {
            .DMA_PeripheralBaseAddr = (uint32_t)&SPI_MASK->DR,
            .DMA_MemoryBaseAddr = 0,
            .DMA_DIR = DMA_DIR_PeripheralDST,
            .DMA_BufferSize = 0,
            .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
            .DMA_MemoryInc = DMA_MemoryInc_Enable,
            .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
            .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
            .DMA_Mode = DMA_Mode_Normal,
            .DMA_Priority = DMA_Priority_Medium,
            .DMA_M2M = DMA_M2M_Disable,
    };
    DMA_Init(SPI_MASK_DMA_CH_TX, &dmaMaskConf);
    DMA_ITConfig(SPI_MASK_DMA_CH_TX, DMA_IT_TC, ENABLE);

    osd.pxClk = getSpiClk(SPI_LEVEL);
    osd.pxPeriod = (1000000000UL /  osd.pxClk) + 1;
    osd.fMaskBuffer = &maskBuffer[1][0];
    osd.fLevelBuffer = &levelBuffer[1][0];
    osd.bMaskBuffer = &maskBuffer[0][0];
    osd.bLevelBuffer = &levelBuffer[0][0];
    osd.swapRequest = 0;
    osd.linecounter = 0;
    osd.lineInProgress = 0;
    osd.tvsysForced = 0;
    osd.waitNextField = 1;

    memset(levelBuffer, 0x00, sizeof(levelBuffer));
    memset(maskBuffer, 0xFF, sizeof(maskBuffer));

    switchTvSystem(&tvPal);

    prepareLine(0);
    prepareLinePeriphs();

    dprint("pxclk = %u", osd.pxClk);
    dprint("pxperiod = %u", osd.pxPeriod);
    dprint("hres = %u", osd.hres);
    dprint("vres = %u", osd.vres);
    dprint("SPICR2 = 0x%02x", SPI_LEVEL->CR2);

    if (osd.tvsysForced)
        osd.state = STATE_RUNNING;
    else
        osd.state = STATE_SYSTEM_DETECTION;
    return OSD_DEVICE_SUCCESS;
}

static uint16_t deviceGetHRes(void *priv)
{
    (void) priv;
    return osd.hres;
}

static uint16_t deviceGetVRes(void *priv)
{
    (void) priv;
    return osd.vres;
}

static void deviceSwapBuffers(void *priv)
{
    (void) priv;
    osd.swapRequest = 1;
}

static int deviceRegisterSwappedCb(void *priv, BufferSwappedCb cb, void *ctx)
{
    (void) priv;
    osd.cbctx = ctx;
    osd.bufferSwappedCb = cb;
    return OSD_DEVICE_SUCCESS;
}

static uint32_t *deviceGetLevelBackBuffer(void *priv)
{
    (void) priv;
    return osd.bLevelBuffer;
}

static uint32_t *deviceGetMaskBackBuffer(void *priv)
{
    (void) priv;
    return osd.bMaskBuffer;
}

OsdDevice spiOsdDevice = {
        .priv = &osd,
        .ops = {
                .start = deviceStart,
                .stop = deviceStop,
                .getHRes = deviceGetHRes,
                .getVRes = deviceGetVRes,
                .getMaskBackBuffer = deviceGetMaskBackBuffer,
                .getLevelBackBuffer = deviceGetLevelBackBuffer,
                .swapBuffers = deviceSwapBuffers,
                .registerBufferSwappedCb = deviceRegisterSwappedCb,
        },
};
