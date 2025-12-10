#include <stdint.h>
#include <stdbool.h>
#include <kernel/exceptions.h>
#include <kernel/exception_print.h>
#include <kernel/kprintf.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/bcm2835_irq.h>
#include <kernel/panic.h>
#include <kernel/scheduler.h>
#include <arch/cpu/cpu.h>
#include <kernel/syscall.h>
#include <user/main.h>
#include <kernel/exc_triggers.h>

bool irq_debug = false;

void und_handler_c(struct exc_frame *frame) {

    if (is_user_mode(frame->spsr)) {
        print_exception_infos(EXC_UND, frame);
        scheduler_on_thread_exit(frame);
    } else {
        print_exception_infos(EXC_UND, frame);
        panic("Undefined Instruction in kernel mode");
    }
    
}

void svc_handler_c(struct exc_frame *frame) {
    if (!is_user_mode(frame->spsr)) {
        print_exception_infos(EXC_SVC, frame);
        panic("Supervisor Call in kernel mode");
    }

    uint32_t nr = decode_svc_imm(frame->lr);

    switch (nr)
    {
    case 0:
        scheduler_on_thread_exit(frame);
        break;
    
    default:
        scheduler_on_thread_exit(frame);
        break;
    }
   
}

void pabt_handler_c(struct exc_frame *frame) {
    if (is_user_mode(frame->spsr)) {
        print_exception_infos(EXC_PABT, frame);
        scheduler_on_thread_exit(frame);
    } else {
        print_exception_infos(EXC_PABT, frame);
        panic("Prefetch Abort in kernel mode");
    }
    
}

void dabt_handler_c(struct exc_frame *frame) {
    if (is_user_mode(frame->spsr)) {
        print_exception_infos(EXC_DABT, frame);
        scheduler_on_thread_exit(frame);
    } else {
        print_exception_infos(EXC_DABT, frame);
        panic("Data Abort in kernel mode");
    }
    
}

void irq_handler_c(struct exc_frame *frame) {
    uint32_t pending1 = irqctrl_pending1();
    uint32_t pending2 = irqctrl_pending2();
    if (irq_debug) {
        print_exception_infos(EXC_IRQ, frame);
    }
    
    if (pending1 & IRQCTRL_TIMER_C1_BIT) {
        uart_putc('!');
        clear_timer_interrupt();
        set_next_timer_interrupt();
        scheduler_on_timer(frame);
    }
    
    if (pending2 & IRQCTRL_PL011_BIT) {
        if (uart_irq_rx_pending()) {
            uart_irq_service_rx();
            
            while (!is_ring_empty()) {
                char c = uart_getc();
                switch (c)
                {
                case 'S':
                    do_svc();
                    break;
                case 'P':
                    do_prefetch_abort();
                    break;
                case 'A':
                    do_data_abort();
                    break;
                case 'U':
                    do_undefined_inst();
                    break;
                default:
                    scheduler_thread_create(main, &c, sizeof(c));
                    if (scheduler_curr()->is_idle) {
                        scheduler_on_timer(frame);
                    }
                    break;
                }
            }
        } 
    }
}