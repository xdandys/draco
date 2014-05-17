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
 * @file    vectors.c
 * @brief   brief description here
 *
 */
#include <stm32f30x.h>
#include "core_cm4.h"
#include "debug.h"

extern unsigned long _estack;

void Default_Handler(void);

void __attribute__ ((weak, naked)) Reset_Handler(void);
void __attribute__ ((weak)) NMI_Handler(void);
void __attribute__ ((weak)) HardFault_Handler(void);
void __attribute__ ((weak)) MemManage_Handler(void);
void __attribute__ ((weak)) BusFault_Handler(void);
void __attribute__ ((weak)) UsageFault_Handler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) SVC_Handler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DebugMon_Handler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) PendSV_Handler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) SysTick_Handler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) WWDG_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) PVD_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TAMP_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) RTC_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) FLASH_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) RCC_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI0_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI2_TS_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel5_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel6_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA1_Channel7_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) ADC1_2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USB_HP_CAN1_TX_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USB_LP_CAN1_RX0_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) CAN1_RX1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) CAN1_SCE_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI9_5_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM1_BRK_TIM15_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM1_UP_TIM16_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM1_TRG_COM_TIM17_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM1_CC_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) I2C1_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) I2C1_ER_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) I2C2_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) I2C2_ER_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) SPI1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) SPI2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USART1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USART2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USART3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) EXTI15_10_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) RTC_ALARM_IT_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USBWakeUp_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM8_BRK_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM8_UP_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM8_TRG_COM_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM8_CC_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) ADC3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) SPI3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) UART4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) UART5_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM6_DAC_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) TIM7_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA2_Channel1_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA2_Channel2_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA2_Channel3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA2_Channel4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) DMA2_Channel5_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) ADC4_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) COMP1_2_3_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) COMP4_5_6_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) COMP7_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USB_HP_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USB_LP_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) USBWakeUp_RMP_IRQHandler(void);
void __attribute__ ((weak, alias ("Default_Handler"))) FPU_IRQHandler(void);



typedef void( *const intfunc )( void );
__attribute__ ((section(".vectors"), used))
void (* const g_pfnVectors[])(void) = {
    (intfunc)((unsigned long)&_estack), /* The stack pointer after relocation */
    Reset_Handler,              /* Reset Handler */
    NMI_Handler,                /* NMI Handler */
    HardFault_Handler,          /* Hard Fault Handler */
    MemManage_Handler,          /* MPU Fault Handler */
    BusFault_Handler,           /* Bus Fault Handler */
    UsageFault_Handler,         /* Usage Fault Handler */
    0,                          /* Reserved */
    0,                          /* Reserved */
    0,                          /* Reserved */
    0,                          /* Reserved */
    SVC_Handler,                /* SVCall Handler */
    DebugMon_Handler,           /* Debug Monitor Handler */
    0,                          /* Reserved */
    PendSV_Handler,             /* PendSV Handler */
    SysTick_Handler,            /* SysTick Handler */

    /* External Interrupts */
    WWDG_IRQHandler,
    PVD_IRQHandler,             /* PVD through EXTI Line detect */
    TAMP_IRQHandler,
    RTC_IRQHandler,             /* RTC */
    FLASH_IRQHandler,           /* Flash */
    RCC_IRQHandler,             /* RCC */
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_TS_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Channel1_IRQHandler,
    DMA1_Channel2_IRQHandler,
    DMA1_Channel3_IRQHandler,
    DMA1_Channel4_IRQHandler,
    DMA1_Channel5_IRQHandler,
    DMA1_Channel6_IRQHandler,
    DMA1_Channel7_IRQHandler,
    ADC1_2_IRQHandler,
    USB_HP_CAN1_TX_IRQHandler,
    USB_LP_CAN1_RX0_IRQHandler,
    CAN1_RX1_IRQHandler,
    CAN1_SCE_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_TIM15_IRQHandler,
    TIM1_UP_TIM16_IRQHandler,
    TIM1_TRG_COM_TIM17_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    USART3_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_ALARM_IT_IRQHandler,
    USBWakeUp_IRQHandler,
    TIM8_BRK_IRQHandler,
    TIM8_UP_IRQHandler,
    TIM8_TRG_COM_IRQHandler,
    TIM8_CC_IRQHandler,
    ADC3_IRQHandler,
    0,
    0,
    0,
    SPI3_IRQHandler,
    UART4_IRQHandler,
    UART5_IRQHandler,
    TIM6_DAC_IRQHandler,
    TIM7_IRQHandler,
    DMA2_Channel1_IRQHandler,
    DMA2_Channel2_IRQHandler,
    DMA2_Channel3_IRQHandler,
    DMA2_Channel4_IRQHandler,
    DMA2_Channel5_IRQHandler,
    ADC4_IRQHandler,
    0,
    0,
    COMP1_2_3_IRQHandler,
    COMP4_5_6_IRQHandler,
    COMP7_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    USB_HP_IRQHandler,
    USB_LP_IRQHandler,
    USBWakeUp_RMP_IRQHandler,
    0,
    0,
    0,
    0,
    FPU_IRQHandler,

    // 30x 0x0 to 512B
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};


__attribute__ ((section(".app_size"), used))
const unsigned long __APP_SIZE = 0x00000000;            // will be filled by stampelf

__attribute__ ((section(".app_time_stamp"), used))
const unsigned long __APP_TIME_STAMP = 0x00000000;      // will be filled by stampelf


__attribute__ ((section(".app_crc"), used))
const unsigned long __APP_CRC = 0x00000000;              // will be filled by stampelf


void Default_Handler(void) {
    DebugSendString("!!! Default Handler !!!");
    dprint("0x%x", NVIC->ISPR[0]);
    while (1) {}
}

void NMI_Handler(void)
{
    DebugSendString("!!! NMI exception !!!");
    while(1);
}

void HardFault_Handler(void)
{
    DebugSendString("!!! HardFault exception !!!");
    while(1);
}

void MemManage_Handler(void)
{
    DebugSendString("!!! MemManage exception !!!");
    while(1);
}

void BusFault_Handler(void)
{
    DebugSendString("!!! BusFault exception !!!");
    while(1);
}

void UsageFault_Handler(void)
{
    DebugSendString("!!! UsageFault exception !!!");
    while(1);
}

