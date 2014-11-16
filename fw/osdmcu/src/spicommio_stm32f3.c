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
 * @file    spicommio_stm32f3.c
 * @brief   SPI communication STM32F3 driver
 *
 */

#include "spicommio_stm32f3.h"
#include "debug.h"
#include "board.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_dma.h"
#include "debug.h"
#include <string.h>

#define RX_DMA_BUFFER_SIZE              512
#define TX_DMA_BUFFER_SIZE              384

DMA_MEM static uint8_t rxDmaBuff[RX_DMA_BUFFER_SIZE];
DMA_MEM static uint8_t txDmaBuff[TX_DMA_BUFFER_SIZE];
static uint16_t rxDmaReadPos = 0;
static volatile int irqDisCounter = 0;

static void disableIrq(void)
{
    __set_BASEPRI(15 << (8 - __NVIC_PRIO_BITS));
    irqDisCounter++;
}

static void enableIrq(void)
{
    if (irqDisCounter > 1) {
        irqDisCounter--;
    }
    else {
        irqDisCounter = 0;
        __set_BASEPRI(0U);
    }
}

void SPI_COMMIO_IRQ_HANDLER(void)
{
    SPI_COMMIO->SR;
    uint16_t writePos = RX_DMA_BUFFER_SIZE - SPI_COMMIO_DMA_CH_RX->CNDTR;
    while(rxDmaReadPos != writePos) {
        uint8_t byte = rxDmaBuff[rxDmaReadPos++];
        if (rxDmaReadPos >= RX_DMA_BUFFER_SIZE) rxDmaReadPos = 0;
        if (spiCommIo.cfg->onByteReceived != 0)
            spiCommIo.cfg->onByteReceived(spiCommIo.cfg->priv, byte);
    }
}

void SPI_COMMIO_DMA_IRQ_HANDLER(void)
{
    if (SPI_COMMIO_DMA->ISR & (DMA_ISR_TCIF1 << (4 * (SPI_COMMIO_DMA_CH_TX_N - 1))))
    {
        SPI_COMMIO_DMA_CH_TX->CCR &= ~DMA_CCR_EN;
        SPI_COMMIO_DMA->IFCR = DMA_IFCR_CTCIF1 << (4 * (SPI_COMMIO_DMA_CH_TX_N - 1));
        spiCommIo.cfg->onTxDone(spiCommIo.cfg->priv);
    }
}

void spiCommTx(const uint8_t *data, uint16_t len)
{
    if ((len + 4) > TX_DMA_BUFFER_SIZE)
        return;

    memcpy(txDmaBuff, data, len);

    // we use this dirty trick to know about tx done when DMA transfer ends (SPI FIFO has 4 byte depth)
    txDmaBuff[len] = 0x00;
    txDmaBuff[len + 1] = 0x00;
    txDmaBuff[len + 2] = 0x00;
    txDmaBuff[len + 3] = 0x00;

    disableIrq();
    SPI_COMMIO_DMA_CH_TX->CCR &= ~(DMA_CCR_EN | DMA_CCR_CIRC);
    SPI_COMMIO_DMA->IFCR = DMA_IFCR_CTCIF1 << (4 * (SPI_COMMIO_DMA_CH_TX_N - 1));
    NVIC_ClearPendingIRQ(SPI_COMMIO_DMA_CH_TX_IRQN);
    SPI_COMMIO_DMA_CH_TX->CMAR = (uint32_t)txDmaBuff;
    SPI_COMMIO_DMA_CH_TX->CNDTR = len + 4;
    SPI_COMMIO_DMA_CH_TX->CCR |= DMA_CCR_TCIE | DMA_CCR_EN;
    enableIrq();
}

void spiCommIoBlockCb(void)
{
    disableIrq();
}

void spiCommIoUnblockCb(void)
{
    enableIrq();
}

void spiCommIoSetIrq(int en)
{
    if (en)
        gpioControl(SPI_COMMIO_IRQ_GPIO, 0);
    else
        gpioControl(SPI_COMMIO_IRQ_GPIO, 1);
}

void spiCommIoStart(const SpiCommIoConfig *config)
{
    spiCommIo.cfg = config;

    disableIrq();
    rxDmaReadPos = 0;
    SPI_COMMIO_RCC_ENABLE;
    SPI_COMMIO_RCC_RESET;
    SPI_COMMIO->CR2 |= (7 << 8) | SPI_CR2_RXNEIE | SPI_CR2_FRXTH | SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;
    SPI_COMMIO->CR1 |= SPI_CR1_SPE | SPI_CR1_CPOL | SPI_CR1_CPHA;

    // configure SPI RX DMA
    SPI_COMMIO_DMA_CH_RX->CCR = DMA_CCR_CIRC | DMA_CCR_MINC;
    SPI_COMMIO_DMA_CH_RX->CNDTR = RX_DMA_BUFFER_SIZE;
    SPI_COMMIO_DMA_CH_RX->CMAR = (uint32_t)&rxDmaBuff[0];
    SPI_COMMIO_DMA_CH_RX->CPAR = (uint32_t)&SPI_COMMIO->DR;
    SPI_COMMIO_DMA_CH_RX->CCR |= DMA_CCR_EN;

    // configure SPI TX DMA
    SPI_COMMIO_DMA_CH_TX->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
    SPI_COMMIO_DMA_CH_TX->CPAR = (uint32_t)&SPI_COMMIO->DR;

    NVIC_ClearPendingIRQ(SPI_COMMIO_DMA_CH_TX_IRQN);

    NVIC_EnableIRQ(SPI_COMMIO_IRQN);
    NVIC_SetPriority(SPI_COMMIO_IRQN, 15);

    NVIC_EnableIRQ(SPI_COMMIO_DMA_CH_TX_IRQN);
    NVIC_SetPriority(SPI_COMMIO_DMA_CH_TX_IRQN, 15);

    enableIrq();
}

void spiCommIoStop(void)
{
    SPI_COMMIO_DMA_CH_RX->CCR = 0;
    SPI_COMMIO_DMA_CH_TX->CCR = 0;
    SPI_COMMIO_DMA->IFCR = DMA_IFCR_CTCIF1 << (5 * SPI_COMMIO_DMA_CH_TX_N);
    NVIC_DisableIRQ(SPI_COMMIO_DMA_CH_TX_IRQN);
    NVIC_DisableIRQ(SPI_COMMIO_IRQN);
    SPI_COMMIO_RCC_RESET;
}

SpiCommIo spiCommIo = {
       .start = spiCommIoStart,
       .stop = spiCommIoStop,
       .blockCb = spiCommIoBlockCb,
       .unblockCb = spiCommIoUnblockCb,
       .setIrq = spiCommIoSetIrq,
       .tx = spiCommTx,
};
