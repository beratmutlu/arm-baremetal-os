
/**
 * @file kprintf.c
 * @brief Implementation of kprintf using kvprintf and the UART backend.
 */

#include <kernel/kprintf.h>
#include <arch/bsp/uart.h>
#include <lib/kvprintf.h>
#include <stddef.h>


void uart_putc_adapter(char c, void *ctx) {
    (void)ctx;
    uart_putc(c);
}

void kprintf [[gnu::format(printf, 1, 2)]] (const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    kvprintf(uart_putc_adapter, NULL, fmt, ap);
    va_end(ap);
}
