#include <stdint.h>

// UART addresses for Raspberry Pi 4
#define UART_BASE       0xFE201000
#define UART_DR         ((volatile uint32_t*)(UART_BASE + 0x00))
#define UART_FR         ((volatile uint32_t*)(UART_BASE + 0x18))
#define UART_IBRD       ((volatile uint32_t*)(UART_BASE + 0x24))
#define UART_FBRD       ((volatile uint32_t*)(UART_BASE + 0x28))
#define UART_LCRH       ((volatile uint32_t*)(UART_BASE + 0x2C))
#define UART_CR         ((volatile uint32_t*)(UART_BASE + 0x30))

// UART flags
#define UART_FR_TXFF    (1 << 5)  // Transmit FIFO full
#define UART_CR_UARTEN  (1 << 0)  // UART enable
#define UART_CR_TXE     (1 << 8)  // Transmit enable
#define UART_LCRH_WLEN8 (3 << 5)  // 8-bit words

void uart_init(void) {
    // Disable UART
    *UART_CR = 0;
    
    // Set baud rate to 115200
    // Assuming 48MHz clock: 48000000 / (16 * 115200) = 26.041666...
    *UART_IBRD = 26;
    *UART_FBRD = 3;  // Fractional part: 0.041666... * 64 ≈ 3
    
    // Configure: 8 bits, no parity, 1 stop bit
    *UART_LCRH = UART_LCRH_WLEN8;
    
    // Enable UART and transmitter
    *UART_CR = UART_CR_UARTEN | UART_CR_TXE;
}

void uart_putc(char c) {
    // Wait until transmit FIFO is not full
    while (*UART_FR & UART_FR_TXFF) {
        // Wait
    }
    
    // Write character to data register
    *UART_DR = c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r');  // Add carriage return for proper line ending
        }
        uart_putc(*str++);
    }
}

void main(void) {
    // Initialize UART
    uart_init();
    
    // Print our hello world message
    uart_puts("Hello, World from Raspberry Pi RTOS!\n");
    uart_puts("This is a bare-metal kernel running in QEMU\n");
    uart_puts("ARM64 Cortex-A72 processor initialized\n");
    uart_puts("UART communication working!\n");
    uart_puts("\nRTOS Development Environment Ready!\n");
    
    // Keep the kernel alive with a simple loop
    volatile int counter = 0;
    while (1) {
        counter++;
        
        // Print a heartbeat every million iterations
        if (counter % 1000000 == 0) {
            uart_puts("Heartbeat: Kernel is alive!\n");
        }
        
        // Small delay to prevent overwhelming output
        for (volatile int i = 0; i < 10000; i++) {
            // Just wait
        }
    }
}
