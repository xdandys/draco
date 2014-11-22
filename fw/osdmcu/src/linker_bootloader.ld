/*
 * layout for STM32F303CC
 */
MEMORY
{
    flash   (RX)  : org = 0x08000000, len = 16k 
    ram     (RWX) : org = 0x20000000, len = 40k
    ram_ccm (RWX) : org = 0x10000000, len = 8k
}

__ram_start__           = ORIGIN(ram);
__ram_size__            = LENGTH(ram);
__ram_end__             = __ram_start__ + __ram_size__;
__ram_ccm_start__           = ORIGIN(ram_ccm);
__ram_ccm_size__            = LENGTH(ram_ccm);
__ram_ccm_end__             = __ram_ccm_start__ + __ram_ccm_size__;


_estack	 				= __ram_ccm_end__;      /* end of the stack */

SECTIONS
{
    /* application ISR vectors */
    
    _isr_vectors_flash_start = ORIGIN(flash);
    .vectors ORIGIN(flash) : ALIGN(128)
    {
    	_isr_vectors_start = . ;
    	KEEP(*(.vectors));
    	_isr_vectors_end = . ;
    	. += 512 - (_isr_vectors_end - _isr_vectors_start);       /* fixed size section 512B */
    } > flash
    _isr_vectors_flash_end = _isr_vectors_flash_start - (_isr_vectors_end * _isr_vectors_start);
    
    /* application size(filled by stampelf util) */
    .app_size :
    {
    	_sapp_size = .;
    	KEEP(*(.app_size));
    	_eapp_size = .;
    	. += 4 - (_eapp_size - _sapp_size);
    } > flash
    
    /* code and constants*/
    .text : ALIGN(16) SUBALIGN(16)
    {
        _text = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        *(.glue_7t)
        *(.glue_7)
        *(.gcc*)
    } > flash

    .ctors :
    {
        PROVIDE(_ctors_start_ = .);
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(_ctors_end_ = .);
    } > flash

    .dtors :
    {
        PROVIDE(_dtors_start_ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(_dtors_end_ = .);
    } > flash

    .ARM.extab : {*(.ARM.extab* .gnu.linkonce.armextab.*)}

    __exidx_start = .;
    .ARM.exidx : {*(.ARM.exidx* .gnu.linkonce.armexidx.*)} > flash
    __exidx_end = .;

    .eh_frame_hdr : {*(.eh_frame_hdr)}

    .eh_frame : ONLY_IF_RO {*(.eh_frame)}

    /*. = ALIGN(4);*/
    _etext = .;
    _textdata = _etext;

	/* bootloader activity word */
    .blact (NOLOAD) : ALIGN(4)
    {
        _sblact_section = .;
        KEEP(*(.blact_section))
        _eblact_section = .;
        . += 4 - (_eblact_section - _sblact_section);
    } > ram_ccm

    /* initialized data*/
    .data : ALIGN(4)
    {
        _sdata = .;
        *(.data)
        . = ALIGN(4);
        *(.data.*)
        . = ALIGN(4);
        *(.ramtext)
        . = ALIGN(4);
        _edata = .;
    } > ram_ccm AT > flash
    
    
    /* application version string */
    .app_version : ALIGN(4)
    {
		/* fixed size section 32B */
		_sapp_version = .;
		KEEP(*(.app_version));
		_eapp_version = .;		 
		. += 32 - (_eapp_version - _sapp_version);    	
    } > flash
    
    /* application time stamp(filled by stampelf util) */
    .app_time_stamp : ALIGN(4)
    {
    	/* fixed size section 4B */
    	_sapp_time_stamp = .;
    	KEEP(*(.app_time_stamp));
    	_eapp_time_stamp = .;
    	. += 4 - (_eapp_time_stamp - _sapp_time_stamp);
    } > flash

    /* application CRC-32(filled by stampelf util) */
    .app_crc : ALIGN(4)
    {
    	/* fixed size section 4B */
    	_sapp_crc = .;
    	KEEP(*(.app_crc));
    	_eapp_crc = .;
    	. += 4 - (_eapp_crc - _sapp_crc); 
    } > flash
    
    _app_size = SIZEOF(.text) + SIZEOF(.data) + SIZEOF(.app_version) + SIZEOF(.app_crc) + SIZEOF(.app_time_stamp);

    /* uninitialized data */
    .bss :
    {
        _bss_start = .;
        *(.bss)
        . = ALIGN(4);
        *(.bss.*)
        . = ALIGN(4);
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > ram_ccm
    
    .dma_region (NOLOAD) : ALIGN(4) SUBALIGN(4)
    {      
    	_dma_region_start = .;
    	*(.videobuffer)
    	*(.dma)
    	_dma_region_end = .;
    } > ram
}

PROVIDE(end = .);
_end            = .;