/*
 * debug.c
 *
 *  Created on: 23.1.2012
 *      Author: strnad
 */


#include <stm32f30x.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_usart.h>
#include <stm32f30x_gpio.h>
#include <stdint.h>
#include "debug.h"
#include "board.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "stdlib.h"


#define DEBUG_USE_SEMIHOSTING
#define DEBUG_BAUDRATE  115200


#ifdef DEBUG_USART2_PA2_PA3
    #define DEBUG_USART2
    #define USART_DBG USART2
    #define USART_FREQ  SYSCLK_FREQ
#elif defined DEBUG_USART1_PA9_PA10
    #define DEBUG_USART1
    #define USART_DBG USART1
    #define USART_FREQ  (SYSCLK_FREQ)
#endif


void DebugInit(void)
{
#ifdef DEBUG_USART2_PA2_PA3
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL2 | GPIO_AFRL_AFRL3);
    GPIOA->AFR[0] |= ((1 << 8) | (1 << 12));                   // assign PA2, PA3 alternate function to USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    USART_DBG->BRR = USART_FREQ / DEBUG_BAUDRATE;
    USART_DBG->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    dbgready = 1;

#elif defined DEBUG_USART1_PA9_PA10
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2);
    GPIOA->AFR[1] |= ((7 << 4) | (7 << 8));                   // assign PA9, PA10 alternate function to USART1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART_DBG->BRR = USART_FREQ / DEBUG_BAUDRATE;
    USART_DBG->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    dbgready = 1;

#endif

}

int shwrites(const char *str)
{
    static uint32_t args[3];
    uint32_t size = strlen(str);
    args[0] = 1;
    args[1] = (uint32_t)str;
    args[2] = size;

    // return -1;
    if ((CoreDebug->DHCSR & 1) == 0) return -1;

    asm volatile("mov r0, #5\n"
        "mov r1, %0\n"
        "bkpt 0x00ab"  : : "r"(args) : "r0", "r1");
    return args[2];
}

#ifndef DEBUG_USE_SEMIHOSTING
void DebugSendString(const char *str)
{
#if defined DEBUG_USART1_PA9_PA10 || defined DEBUG_USART2_PA2_PA3
    uint16_t i = 0;
    if (!dbgready) DebugInit();
    for(i=0;str[i]!=0x00;i++)
    {
        while((USART_DBG->ISR & USART_ISR_TXE) == 0);
        USART_DBG->TDR = str[i];
    }
    //while ((USART_DBG->SR & USART_SR_TC) == 0);
#endif
}

void DebugPutChar(char c)
{
#if defined DEBUG_USART1_PA9_PA10 || defined DEBUG_USART2_PA2_PA3
    if (!dbgready) DebugInit();
    while((USART_DBG->ISR & USART_ISR_TXE) == 0);
    USART_DBG->TDR = c;
#endif
}
#else
static char shwrbuff[128];
static uint32_t shwrbuffCount = 0;

void DebugPutChar(char c)
{
    shwrbuff[shwrbuffCount++] = c;

    if ((shwrbuffCount > (sizeof(shwrbuff) -2)) || (c == '\n') || (c == 0))  {
        shwrbuff[shwrbuffCount] = 0;
        shwrbuffCount = 0;
        shwrites(shwrbuff);
    }
}

void DebugSendString(const char *msg)
{
    shwrites(msg);
}
#endif

void _dhexprint(const char *file, const char *function, const uint8_t *dataPointer, uint16_t len)
{
    uint8_t i;
    uint16_t pos=0;
    while(1)
    {
        DebugPrintf(DEBUG_STR_FORMAT "%04x ",getElapsedMs() / 1000, getElapsedMs() % 1000, file, function, pos);
        for (i=0;i<16;i++) {
            if (pos<len) DebugPrintf("%02x ", dataPointer[pos++]);
        }
        DebugPrintf("\r\n");
        if(pos>=len) break;
    }
}


void _dbarprint(const char *file, const char *function, uint32_t value, uint32_t rangeMin, uint32_t rangeMax, uint8_t size)
{
    char bar[size + 3];
    bar[0] = '|';
    bar[size+1] = '|';
    bar[size+2] = 0x00;
    uint16_t i;
    for (i=1;i<(size+1); i++) bar[i] = ' ';
    uint8_t pos = (value - rangeMin) / ((rangeMax - rangeMin) / size);
    if (pos <= size)
        bar[pos] = '*';
    DebugPrintf(DEBUG_STR_FORMAT "%u%s%u  %u\r\n",getElapsedMs() / 1000, getElapsedMs() % 1000, file, function, rangeMin, bar, rangeMax, value);
}


static void printchar(char **str, int c)
{
    if (str) {
        **str = c;
        ++(*str);
    }
    else {
        DebugPutChar((char) c);
    }
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for ( ; width > 0; --width) {
            printchar (out, padchar);
            ++pc;
        }
    }
    for ( ; *string ; ++string) {
        printchar (out, *string);
        ++pc;
    }
    for ( ; width > 0; --width) {
        printchar (out, padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    int t, neg = 0, pc = 0;
    unsigned int u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints (out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    while (u) {
        t = u % b;
        if( t >= 10 )
            t += letbase - '0' - 10;
        *--s = t + '0';
        u /= b;
    }

    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            printchar (out, '-');
            ++pc;
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (out, s, width, pad);
}

static int print(char **out, const char *format, va_list args )
{
    int width, pad;
    int pc = 0;
    char scr[2];

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0') break;
            if (*format == '%') goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            if( *format == 's' ) {
                char *s = (char *)va_arg( args, int );
                pc += prints (out, s?s:"(null)", width, pad);
                continue;
            }
            if( *format == 'd' ) {
                pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
                continue;
            }
            if( *format == 'x' ) {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'X' ) {
                pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
                continue;
            }
            if( *format == 'u' ) {
                pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'c' ) {
                /* char are converted to int then pushed on the stack */
                scr[0] = (char)va_arg( args, int );
                scr[1] = '\0';
                pc += prints (out, scr, width, pad);
                continue;
            }
        }
        else {
        out:
            printchar (out, *format);
            ++pc;
        }
    }
    if (out) **out = '\0';
    va_end( args );
    return pc;
}

int DebugPrintf(const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( 0, format, args );
}

int sprintf(char *out, const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return print( &out, format, args );
}




