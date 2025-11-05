
/**
 * @file kprintf.c
 * @brief Implementation of kprintf using kvprintf and the UART backend.
 *
 * Bridges kvprintf's byte-emitter interface to the UART driver and provides
 * a convenient, printf-like function.
 *
 * @ingroup kprintf_api
 */

#include <kernel/kprintf.h>
#include <arch/bsp/uart.h>
#include <lib/kvprintf.h>
#include <stddef.h>


static void uart_putc_adapter(char c, void *ctx);


/** @copydoc kprintf */
void kprintf(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    kvprintf(uart_putc_adapter, NULL, fmt, ap);
    va_end(ap);
}

/**
 * @brief Adapter from kvprintf's kputc_fn to the UART driver.
 * Forwards a single character to @ref uart_putc. @p ctx is unused for now.
 */
static void uart_putc_adapter(char c, void *ctx) {
    (void)ctx;
    uart_putc(c);
}