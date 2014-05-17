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


//void __copyVectorsToRAM(void) {
//    unsigned long *src, *dst;
//    src = &_isr_vectors_flash_start;
//    dst = &_isr_vectors_start;
//
//    if (src != dst)
//        while(dst < &_isr_vectors_end)
//            *(dst++) = *(src++);
//}

void Reset_Handler(void) {
	__init_Data();
    // backup bootloader activity word and remap vectors to RAM
    unsigned long __bl_act;
	__bl_act = *(volatile unsigned long*)0x20000000;
	// __copyVectorsToRAM();
	*(volatile unsigned long*)0x20000000 = __bl_act;

	// vectors are now in RAM
	// SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;

	SCB->VTOR = (uint32_t)&_isr_vectors_start;
	setupClocks();

	// enable DWT
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	main();
	while(1) {}
}

