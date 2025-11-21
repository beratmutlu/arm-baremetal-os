#include <stdint.h>
#include <stdbool.h>
#include <kernel/exceptions.h>
#include <kernel/exception_print.h>
#include <kernel/kprintf.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/bcm2835_irq.h>
#include <kernel/panic.h>

bool irq_debug = false;

void und_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_UND, frame);
    panic("Undefined Instruction");
}

void svc_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_SVC, frame);
    panic("Supervisor Call");
}

void pabt_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_PABT, frame);
    panic("Prefetch Abort");
}

void dabt_handler_c[[noreturn]](struct exc_frame *frame) {
    print_exception_infos(EXC_DABT, frame);
    panic("Data Abort");
}

void irq_handler_c(struct exc_frame *frame) {
    uint32_t pending1 = irqctrl_pending1();
    uint32_t pending2 = irqctrl_pending2();
    if (irq_debug) {
        print_exception_infos(EXC_IRQ, frame);
    }
    
    if (pending1 & IRQCTRL_TIMER_C1_BIT) {
        kprintf("!\n");
        clear_timer_interrupt();
        set_next_timer_interrupt();
        
    }
    
    if (pending2 & IRQCTRL_PL011_BIT) {
        if (uart_irq_rx_pending()) {
            uart_irq_service_rx();
        }
    }
}