
/**
 * @file kprintf.c
 * @brief Implementation of kprintf using kvprintf and the UART backend.
 */

#include <kernel/kprintf.h>
#include <arch/bsp/uart.h>
#include <lib/kvprintf.h>
#include <stddef.h>


/**
 * @brief Adapter from kvprintf's kputc_fn to the UART driver.
 * Forwards a single character to @ref uart_putc. @p ctx is unused for now.
 */
static void uart_putc_adapter(char c, void *ctx) {
    (void)ctx;
    uart_putc(c);
}

void kprintf [[gnu::format(printf, 1, 2)]] (const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    kvprintf(uart_putc_adapter, NULL, fmt, ap);
    va_end(ap);
}
