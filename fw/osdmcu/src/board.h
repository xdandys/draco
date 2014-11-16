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
 * @file    board.h
 * @brief   board specific defines goes here
 *
 */
#ifndef HW_H_
#define HW_H_

#define XTAL_FREQ       8000000UL
#define SYSCLK_FREQ     56000000UL

#include <stm32f30x.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_spi.h>

#define VIDEO_BUFFER_MEM __attribute__ ((section(".videobuffer")))
#define DMA_MEM __attribute__ ((section(".dma")))

#define SPI_LEVEL                   SPI2
#define SPI_MASK                    SPI1
#define SPI_LEVEL_RCC_ENABLE        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
#define SPI_LEVEL_RCC_DISABLE       RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN;
#define SPI_MASK_RCC_ENABLE         RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
#define SPI_MASK_RCC_DISABLE        RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
#define SPI_LEVEL_RCC_RESET         {RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;}
#define SPI_MASK_RCC_RESET          {RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;}
#define SPI_MASK_DMA                DMA1
#define SPI_LEVEL_DMA               DMA1
#define SPI_LEVEL_DMA_CH_TX         DMA1_Channel5
#define SPI_LEVEL_DMA_CH_TX_N       5
#define SPI_MASK_DMA_CH_TX          DMA1_Channel3
#define SPI_MASK_DMA_CH_TX_N        3
#define SPI_LEVEL_DMA_CH_IRQH       DMA1_Channel5_IRQHandler
#define SPI_MASK_DMA_CH_IRQH        DMA1_Channel3_IRQHandler
#define SPI_COMMIO                  SPI3
#define SPI_COMMIO_RCC_ENABLE       RCC->APB1ENR |= RCC_APB1ENR_SPI3EN
#define SPI_COMMIO_RCC_DISABLE      RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN;
#define SPI_COMMIO_RCC_RESET        {RCC->APB1RSTR |= RCC_APB1RSTR_SPI3RST;RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;}
#define SPI_COMMIO_DMA              DMA2
#define SPI_COMMIO_DMA_CH_TX        DMA2_Channel2
#define SPI_COMMIO_DMA_CH_RX        DMA2_Channel1
#define SPI_COMMIO_DMA_CH_TX_N      2
#define SPI_COMMIO_DMA_CH_RX_N      1
#define SPI_COMMIO_DMA_CH_TX_IRQN   DMA2_Channel2_IRQn
#define SPI_COMMIO_DMA_CH_RX_IRQN   DMA2_Channel1_IRQn
#define SPI_COMMIO_IRQN             SPI3_IRQn
#define SPI_COMMIO_IRQ_GPIO         PinSpiInt
#define SPI_COMMIO_IRQ_HANDLER      SPI3_IRQHandler
#define SPI_COMMIO_DMA_IRQ_HANDLER  DMA2_Channel2_IRQHandler

typedef enum GPIOAliasEnum {

    PinOsdPixelClockIn,
    PinOsdMask,
    PinOsdSyncOut,
    PinLedRed,
    PinLedOrange,
    PinLedGreen,
    PinLedBlue,
    PinSpiCs,
    PinSpiSck,
    PinSpiMiso,
    PinSpiMosi,
    PinExtUart2Tx,
    PinExtUart2Rx,
    PinOsdCSync,
    PinOsdVSync,
    PinOsdPixelClockOut,
    PinOsdHSync,
    PinOsdVout,
    PinSpiInt,
    NoPin,
} GpioAlias;

/*
 * Communication with bootloader
 */
#define BL_ACT_APPTOBL                  0xAA550000
#define BL_ACT_BLTOAPP                  0x55AA0000
#define BL_ACT_APPREQ_STAY              0x00000001  /**< application request to stay in bootloader for unlimited time */
#define BL_ACT_APPREQ_STAY_5S           0x00000002  /**< application request to stay in bootloader for 5 sec */
#define BL_ACT_BL_ACTIVITY              0x00000100  /**< bootloader was active */
#define BL_ACT_BL_FLASH                 0x00000200  /**< bootloader was active and flashing was performed */

void setBlAct(uint32_t blAct);
uint32_t getBlAct(void);

void boardInit(void);
void jumpToBootLoader(uint16_t flags);

void gpioControl(GpioAlias alias, uint8_t on);
uint8_t gpioState(GpioAlias alias);

void disableIrqs(void);
void enableIrqs(void);

void setupClocks(void);

#endif /* HW_H_ */
