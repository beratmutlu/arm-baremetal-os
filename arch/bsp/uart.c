
/**
 * @file uart.c
 * @brief UART driver built on PL011 BSP with interrupt-driven receive.
 *
 * Implements the API defined in uart.h using:
 * - Blocking transmit (busy-wait)
 * - Interrupt-driven receive (ring buffer)
 *
 * @ingroup uart_api
 */

#include <arch/bsp/uart.h>
#include <arch/bsp/pl011_regs.h>
#include <lib/ringbuffer.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <config.h>
#include <arch/bsp/irqctrl.h>
#include <arch/cpu/cpu.h>

/** @brief Ring buffer for received characters (size from config.h). */
create_ringbuffer(ring, UART_INPUT_BUFFER_SIZE);

/** @copydoc uart_irq_enable */
void uart_irq_enable(void) {
    /* Enable RX interrupt only (no TX or RT). */
    PL011->ICR  = 0x7FFu;      /* Clear all pending interrupts.   */
    PL011->IMSC = (1u << 4);   /* Enable RX interrupt only.       */
}

/** @copydoc uart_irq_rx_pending */
bool uart_irq_rx_pending(void){
    return (PL011->MIS & (1 << 4)) != 0;
}

/** @copydoc uart_irq_service_rx */
void uart_irq_service_rx(void){
    while (!pl011_rx_empty()) {
        char c = (char)(uint8_t)PL011->DR;
        buff_putc(ring, c);
    }
    PL011->ICR = 1 << 4;
}

/** @copydoc uart_init */
void uart_init(void) {
    PL011->RSR_ECR = 0xFFFFFFFFu;   /* Clear all error flags */
    PL011->IMSC = 1 << 4;           /* Enable RX interrupt */
    PL011->ICR = 1 << 4;            /* Clear any pending RX interrupt */
}
/** @copydoc uart_putc */
void uart_putc(char c) {
    while (pl011_tx_full()) { /* busy wait */ }
    PL011->DR = (uint32_t)(uint8_t)c;
}

/** @copydoc uart_getc */
char uart_getc(void) {
    return buff_getc(ring);
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

