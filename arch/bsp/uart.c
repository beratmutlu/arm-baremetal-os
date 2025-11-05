
/**
 * @file uart.c
 * @brief Simple blocking UART driver built on PL011 BSP.
 *
 * Implements the API defined in uart.h using busy-wait.
 * 
 * @ingroup uart_api
 */

#include <arch/bsp/uart.h>
#include <arch/bsp/pl011_regs.h>
#include <stdint.h>

/** @copydoc uart_init */
void uart_init(void) {
    /* On QEMU the UART is already configured, so we just clear sticky errors. */
    PL011->RSR_ECR = 0xFFFFFFFFu;
}

/** @copydoc uart_putc */
void uart_putc(char c) {
    while (pl011_tx_full()) { /* busy wait */ }
    PL011->DR = (uint32_t)(uint8_t)c;
}

/** @copydoc uart_getc */
char uart_getc(void) {
    while (pl011_rx_empty()) { /* busy wait*/ }
    char c = (char)(uint8_t)PL011->DR;
    return c;
}

/** @copydoc uart_puts */
void uart_puts(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        uart_putc(*s++);
    }
}
