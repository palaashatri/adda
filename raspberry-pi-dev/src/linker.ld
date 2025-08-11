/* Linker script for ARM64 Raspberry Pi kernel */

ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 0x80000 for ARM64 */
    . = 0x80000;
    
    /* Code section */
    .text.boot : {
        *(.text.boot)
    }
    
    .text : {
        *(.text)
    }
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
    }
    
    /* Data section */
    .data : {
        *(.data)
    }
    
    /* BSS section for uninitialized data */
    . = ALIGN(8);
    __bss_start = .;
    .bss : {
        *(.bss)
    }
    __bss_end = .;
    
    __bss_size = (__bss_end - __bss_start) >> 3;
}
