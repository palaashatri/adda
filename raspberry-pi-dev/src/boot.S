// boot.S - Assembly bootstrap for ARM64 Raspberry Pi
// This is the entry point of our kernel

.section ".text.boot"

.globl _start

_start:
    // Get processor ID - we only want to run on core 0
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, core0
    
    // If not core 0, put this core to sleep
halt:
    wfe
    b       halt

core0:
    // Set up stack pointer
    ldr     x1, =_start
    mov     sp, x1
    
    // Clear BSS section
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
    
clear_bss:
    cbz     w2, run_main
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, clear_bss
    
run_main:
    // Jump to C main function
    bl      main
    
    // If main returns, halt the processor
    b       halt
