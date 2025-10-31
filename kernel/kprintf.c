#include <lib/kvprintf.h>
#include <arch/bsp/uart.h>
#include <kernel/kprintf.h>
#include <stddef.h>

static void uart_putc_adapter(char c, void *ctx) {
    (void)ctx;
    uart_putc(c);
}

void kprintf(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    kvprintf(uart_putc_adapter, NULL, fmt, ap);
    va_end(ap);
}
