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
 * @file    board.c
 * @brief   board specific defines goes here
 *
 */

#include <stm32f30x_rcc.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_dma.h>
#include <stm32f30x_pwr.h>
#include <stm32f30x_rtc.h>
#include <stm32f30x_exti.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_adc.h>
#include <stm32f30x_usart.h>
#include <core_cm4.h>

#include "board.h"
#include "debug.h"
#include "delay.h"
#include "version.h"

typedef enum GpioLogicEnum
{
    NonInverted,
    Inverted,
} GpioLogic;

typedef enum GpioExtiConfigEnum
{
    GpioExtiNone,
    GpioExtiRising,
    GpioExtiFalling,
    GpioExtiBothEdges,
} GpioExtiConfig;

typedef struct GPIOInitTableItemStruct {
    GPIO_TypeDef* port;                 /**< pointer to port register */
    uint8_t altFunc;                    /**< GPIO alternate fucntion assignment (0..15) */
    uint8_t pinNr;                      /**< pin number (0..15) */
    const GPIO_InitTypeDef initStruct;  /**< initialization struct */
    uint8_t defaultOutputState;         /**< when configured as output */
    GpioLogic logic;                    /**< select between invereted and noninverted logic */
    GpioExtiConfig exti;                /** exti line configuration */

} GpioItem;

/** nested irq enable/disable counter */
static volatile uint32_t irqDisCnt = 0;

static const GpioItem GPIOInitTable[] = {
        [PinOsdPixelClockIn] =
        {
                .port = GPIOA,
                .pinNr = 5,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_5,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 5
        },

        [PinOsdMask] =
        {
                .port = GPIOA,
                .pinNr = 6,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_6,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 5
        },

        [PinOsdSyncOut] =
        {
                .port = GPIOA,
                .pinNr = 8,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_8,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 1,
                .logic = NonInverted,
                .altFunc = 0
        },

        [PinLedRed] =
        {
                .port = GPIOA,
                .pinNr = 9,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_9,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0
        },

        [PinLedOrange] =
        {
                .port = GPIOA,
                .pinNr = 10,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_10,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0
        },

        [PinLedGreen] =
        {
                .port = GPIOA,
                .pinNr = 11,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_11,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0
        },

        [PinLedBlue] =
        {
                .port = GPIOA,
                .pinNr = 12,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_12,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0
        },

        [PinSpiCs] =
        {
                .port = GPIOA,
                .pinNr = 15,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_15,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 6
        },

        [PinSpiSck] =
        {
                .port = GPIOB,
                .pinNr = 3,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_3,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 6
        },

        [PinSpiMiso] =
        {
                .port = GPIOB,
                .pinNr = 4,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_4,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 6
        },

        [PinSpiMosi] =
        {
                .port = GPIOB,
                .pinNr = 5,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_5,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 6
        },

        [PinExtUart2Tx] =
        {
                .port = GPIOB,
                .pinNr = 6,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_6,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 7
        },

        [PinExtUart2Rx] =
        {
                .port = GPIOB,
                .pinNr = 7,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_7,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 7
        },

        [PinOsdCSync] =
        {
                .port = GPIOB,
                .pinNr = 11,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_11,
                        .GPIO_Mode = GPIO_Mode_IN,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0,
                // .exti = GpioExtiFalling,
        },

        [PinOsdVSync] =
        {
                .port = GPIOB,
                .pinNr = 12,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_12,
                        .GPIO_Mode = GPIO_Mode_IN,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0,
                .exti = GpioExtiFalling,
        },

        [PinOsdPixelClockOut] =
        {
                .port = GPIOB,
                .pinNr = 13,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_13,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 5
        },

        [PinOsdHSync] =
        {
                .port = GPIOB,
                .pinNr = 14,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_14,
                        .GPIO_Mode = GPIO_Mode_IN,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 0,
                .exti = GpioExtiRising,
        },

        [PinOsdVout] =
        {
                .port = GPIOB,
                .pinNr = 15,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_15,
                        .GPIO_Mode = GPIO_Mode_AF,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_UP,
                },
                .defaultOutputState = 0,
                .logic = NonInverted,
                .altFunc = 5
        },

        [PinSpiInt] =
        {
                .port = GPIOC,
                .pinNr = 13,
                .initStruct = {
                        .GPIO_Pin = GPIO_Pin_13,
                        .GPIO_Mode = GPIO_Mode_OUT,
                        .GPIO_Speed = GPIO_Speed_50MHz,
                        .GPIO_OType = GPIO_OType_PP,
                        .GPIO_PuPd = GPIO_PuPd_NOPULL,
                },
                .defaultOutputState = 1,
                .logic = NonInverted,
                .altFunc = 0,
        },

        {0},
};

/** get message from bootloader */
uint32_t getBlAct(void)
{
    return *(volatile unsigned long*)0x20000000;
}

/** set message for bootloader */
void setBlAct(uint32_t blAct)
{
    *(volatile unsigned long*)0x20000000 = blAct;
}

/** enable clocks for all used peripherals */
static void initPeriphClocks(void)
{

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN
                 | RCC_APB1ENR_PWREN
                 | RCC_APB1ENR_TIM3EN
                 | RCC_APB1ENR_TIM4EN
                 | RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN
                 | RCC_APB2ENR_USART1EN
                 | RCC_APB2ENR_SYSCFGEN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN
                |  RCC_AHBENR_DMA2EN
                |  RCC_AHBENR_GPIOAEN
                |  RCC_AHBENR_GPIOBEN
                |  RCC_AHBENR_GPIOCEN;
}

static void initInterrupts(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xffffffff;
        NVIC->ICPR[i] = 0xffffffff;
    }

    NVIC_SetPriorityGrouping(0);

    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    __enable_irq();
}

static void initGPIOs(void)
{
    uint8_t i;

    for (i=0; GPIOInitTable[i].port != 0; i++) {
        const GpioItem *gpio = &GPIOInitTable[i];
        GPIO_Init(gpio->port, (GPIO_InitTypeDef*)&gpio->initStruct);
        if (gpio->initStruct.GPIO_Mode == GPIO_Mode_OUT)
            GPIO_WriteBit(gpio->port, gpio->initStruct.GPIO_Pin,
                    (gpio->logic == NonInverted) ? gpio->defaultOutputState : !gpio->defaultOutputState);

        if (gpio->pinNr < 8) {
            gpio->port->AFR[0] |= gpio->altFunc << (4 * gpio->pinNr);
        } else {
            gpio->port->AFR[1] |= gpio->altFunc << (4 * (gpio->pinNr - 8));
        }

        switch(gpio->exti) {
        case GpioExtiFalling:
            EXTI->FTSR |=  1 << (uint32_t)gpio->pinNr;
            EXTI->RTSR &=  ~(1 << (uint32_t)gpio->pinNr);
            break;

        case GpioExtiRising:
            EXTI->RTSR |=  1 << (uint32_t)gpio->pinNr;
            EXTI->FTSR &=  ~(1 << (uint32_t)gpio->pinNr);
            break;

        case GpioExtiBothEdges:
            EXTI->RTSR |=  1 << (uint32_t)gpio->pinNr;
            EXTI->FTSR |=  1 << (uint32_t)gpio->pinNr;
            break;

        case GpioExtiNone:
        default:
            EXTI->FTSR &=  ~(1 << (uint32_t)gpio->pinNr);
            EXTI->RTSR &=  ~(1 << (uint32_t)gpio->pinNr);
            EXTI->IMR  &=  ~(1 << (uint32_t)gpio->pinNr);
            break;
        }

        if (gpio->exti != GpioExtiNone) {
            EXTI->PR |=  1 << (uint32_t)gpio->pinNr;
            EXTI->IMR  |=  1 << (uint32_t)gpio->pinNr;

            uint32_t exticrVal = 0;
            switch((uint32_t)gpio->port) {
            case (uint32_t)GPIOA:
                exticrVal = 0;
                break;

            case (uint32_t)GPIOB:
                exticrVal = 1;
                break;

            case (uint32_t)GPIOC:
                exticrVal = 2;
                break;

            case (uint32_t)GPIOD:
                exticrVal = 3;
                break;

            case (uint32_t)GPIOE:
                exticrVal = 4;
                break;

            default:
                break;
            }

            SYSCFG->EXTICR[gpio->pinNr / 4] &= ~(0x0f << ((gpio->pinNr % 4) * 4));
            SYSCFG->EXTICR[gpio->pinNr / 4] |= exticrVal << ((gpio->pinNr % 4) * 4);
        }
    }
}

void boardInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    initPeriphClocks();
    initGPIOs();
    initInterrupts();
}

void gpioControl(GpioAlias alias, uint8_t on)
{
    on = (GPIOInitTable[alias].logic == NonInverted) ? on : !on;
    GPIO_WriteBit(GPIOInitTable[alias].port, GPIOInitTable[alias].initStruct.GPIO_Pin, on);
}

uint8_t gpioState(GpioAlias alias)
{
    uint8_t bit = GPIO_ReadInputDataBit(GPIOInitTable[alias].port, GPIOInitTable[alias].initStruct.GPIO_Pin);
    bit = (GPIOInitTable[alias].logic == NonInverted) ? bit : !bit;
    return bit;
}

/** Setup system PLL, clocks and flash prefetching */
void setupClocks(void)
{
    /* FPU settings ------------------------------------------------------------*/
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
      SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
    #endif

    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register */
    RCC->CFGR &= 0xF87FC00C;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

    /* Reset USARTSW[1:0], I2CSW and TIMs bits */
    RCC->CFGR3 &= (uint32_t)0xFF00FCCC;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/
    /* Enable HSE */

    //RCC->CR |= ((uint32_t)RCC_CR_HSEBYP); // for external OCSILATOR !!!
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait till HSE is ready and if Time out is reached exit */
    do
    {
      HSEStatus = RCC->CR & RCC_CR_HSERDY;
      StartUpCounter++;
    } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
    {
      HSEStatus = (uint32_t)0x01;
    }
    else
    {
      HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01)
    {
      /* Enable Prefetch Buffer and set Flash Latency */
      FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;

      /* HCLK = SYSCLK */
      RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

      /* PCLK = HCLK */
      RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;
      RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;

      /* PLL configuration = HSE * 7 = 56 MHz */
      RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
      RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL7);

      /* Enable PLL */
      RCC->CR |= RCC_CR_PLLON;

      /* Wait till PLL is ready */
      while((RCC->CR & RCC_CR_PLLRDY) == 0)
      {
      }

      /* Select PLL as system clock source */
      RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
      RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

      /* Wait till PLL is used as system clock source */
      while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
      {
      }
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock
           configuration. User can add here some code to deal with this error */
        while(1);
    }

    /* Enable HSI 14MHz for ADC */
//    RCC->CR2 |= RCC_CR2_HSI14ON;
//    while (!(RCC->CR2 & RCC_CR2_HSI14RDY));

}

extern void CSyncInterrupt(void);
extern void HSyncInterrupt(void);
extern void VSyncInterrupt(void);

/** EXTI interrupt handler. It will serve GPIO interrutps,
 * pass them to video driver and clear interrupt flags
 */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1 << 14)) {
        HSyncInterrupt();
        EXTI->PR  = 1 << 14;
    }

    if (EXTI->PR & (1 << 11)) {
        CSyncInterrupt();
        EXTI->PR  = 1 << 11;
    }

    if (EXTI->PR & (1 << 12)) {
        VSyncInterrupt();
        EXTI->PR  = 1 << 12;
    }
}

/** Disable all interrupts
 * nesting calls are allowed
 */
void disableIrqs(void)
{
    __disable_irq();
    irqDisCnt++;
}

/**Eanble all interrupts
 * nesting calls are allowed
 */
void enableIrqs(void)
{

    if (irqDisCnt > 1) {
        irqDisCnt--;
    } else {
        irqDisCnt = 0;
        __enable_irq();
    }
}

/** Jump to bootloader code */
void jumpToBootLoader(uint16_t blFlags)
{
    __disable_irq();
    dprint("jumping to bootloader!");

    RCC_APB1PeriphResetCmd(
            RCC_APB1Periph_USB,
            ENABLE);
    RCC_APB1PeriphResetCmd(
            RCC_APB1Periph_USB,
            DISABLE);

    USART_DeInit(USART1);

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    __set_MSP(*(uint32_t*)0x08000000);
    setBlAct(BL_ACT_APPTOBL | blFlags);

    uint32_t* RHAddr = (uint32_t*)*((uint32_t*)((uint32_t)0x08000000 + 4));
    void (*jmpbl)(void) = (void (*)(void)) RHAddr;
    //bye!
    jmpbl();
    while(1);
}
