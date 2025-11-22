#include <kernel/panic.h>
#include <arch/bsp/uart.h>
#include <kernel/kprintf.h>

void _panic [[noreturn]] (const char *func_name, const char *msg) {
    asm volatile("cpsid if");

    uart_init();

    //kprintf("KERNEL PANIC in %s: %s\n", func_name, msg);
    //kprintf("System halted.\n");

    while (true)
    {
        asm volatile("wfi");
    }
    
}