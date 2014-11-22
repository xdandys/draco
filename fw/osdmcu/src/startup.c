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
 * @file    startup.c
 * @brief   MCU startup code
 *
 */

#include <stm32f30x.h>
#include "core_cm4.h"
#include "debug.h"
#include "board.h"

extern unsigned long _textdata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long  _bss_start;
extern unsigned long  _bss_end;
extern unsigned long _isr_vectors_flash_start;
extern unsigned long _isr_vectors_start;
extern unsigned long _isr_vectors_end;

void __init_Data(void);

extern int main(void);

// CRT0
void __init_Data(void) {
    unsigned long *src, *dst;
    // copy data segment to ram
    src = &_textdata;
    dst = &_sdata;
    if (src != dst)
        while(dst < &_edata)
            *(dst++) = *(src++);

    // zero bss segment
    dst = &_bss_start;
    while(dst < &_bss_end)
        *(dst++) = 0;
}

void Reset_Handler(void) {
	__init_Data();
	SCB->VTOR = (uint32_t)&_isr_vectors_start;
	setupClocks();

	// enable DWT
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	main();
	while(1) {}
}
