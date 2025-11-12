#include <stdint.h>
#include <stdbool.h>
#include <kernel/exceptions.h>
#include <kernel/exception_print.h>
#include <kernel/kprintf.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/irqctrl.h>

bool irq_debug = false;

#define halt()                      \
	do {                        \
		asm("cpsid if");    \
		while (true) {      \
			asm("wfi"); \
		}                   \
	} while (0)

void und_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_UND, frame);
    uart_putc('\4');
    halt();
}

void svc_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_SVC, frame);
    uart_putc('\4');
    halt();
}

void pabt_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_PABT, frame);
    uart_putc('\4');
    halt();
}

void dabt_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_DABT, frame);
    uart_putc('\4');
    halt();
}

void irq_handler_c(struct exc_frame *frame) {
    uint32_t pending = irqctrl_pending1();
    
    if (irq_debug) {
        print_exception_infos(EXC_IRQ, frame);
    }
    
    if (pending & (1 << 1)) {
        clear_timer_interrupt();
        set_next_timer_interrupt();
        kprintf("!\n");
    }
    
    if (uart_irq_rx_pending()) {
        uart_irq_service_rx();
    }
}