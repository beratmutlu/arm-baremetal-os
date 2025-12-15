
#include <arch/bsp/uart.h>
#include <arch/bsp/bcm2835_pl011_regs.h>
#include <lib/ringbuffer.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <config.h>
#include <arch/bsp/bcm2835_irq.h>
#include <arch/cpu/cpu.h>

/** @brief Ring buffer for received characters (size from config.h). */
create_ringbuffer(ring, UART_INPUT_BUFFER_SIZE);

void uart_irq_enable(void) {
    PL011->ICR  = INT_RX;       /* Clear RX interrupts. */
    PL011->IMSC = INT_RX;       /* Enable RX interrupt. */
}

bool uart_irq_rx_pending(void){
    return (PL011->MIS & INT_RX) != 0;
}

bool is_ring_empty() {
    return buff_is_empty(ring);
}

char ring_peek(void) {
    return buff_peekc(ring);
}

void uart_irq_service_rx(void){
    while (!pl011_rx_empty()) {
        char c = (char)(uint8_t)PL011->DR;
        buff_putc(ring, c);
    }
    PL011->ICR = INT_RX;
}

void uart_init(void) {
    PL011->RSR_ECR = 0xFFFFFFFFu;   /* Clear all error flags */
    uart_irq_enable();
}

void uart_putc(char c) {
    while (pl011_tx_full()) { /* busy wait */ }
    PL011->DR = (uint32_t)(uint8_t)c;
}

char uart_getc(void) {
    return buff_getc(ring);
}

void uart_puts(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        uart_putc(*s++);
    }
}

