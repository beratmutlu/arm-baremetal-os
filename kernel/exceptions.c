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
#include <user/main.h>
#include <kernel/exc_triggers.h>
#include <user/syscalls.h>


extern char ld_section_kernel_text;
extern char ld_section_user_text;

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
    frame->lr += 4;
    uint32_t nr = (uint32_t) frame->r[SYSCALL_ID_REG];

    switch (nr)
    {
    case SYSCALL_ID_EXIT: {
        scheduler_on_thread_exit(frame);
        break;
    }

    case SYSCALL_ID_CREATE: {
        void (*f)(void *) = (void (*)(void *))(uintptr_t)frame->r[0];
        void *args        = (void *)(uintptr_t)frame->r[1];
        unsigned arg_size = (unsigned)frame->r[2];

        if (!f) {
            scheduler_on_thread_exit(frame);
            break;
        }
        scheduler_thread_create(f, args, arg_size);
        break;
    }

    case SYSCALL_ID_GETC: {
        if (!is_ring_empty()) {
            char c = uart_getc();
            frame->r[0] = (uint32_t) c;
        } else { 
            scheduler_blocked_on_io(frame);
        }
        break;
    }

    case SYSCALL_ID_PUTC: {
        char c = (char) frame->r[0];
        uart_putc(c);
        break;
    }

    case SYSCALL_ID_SLEEP: {
        unsigned cycles = frame->r[0];
        if (cycles == 0) {
            return;
        }
        scheduler_blocked_on_sleep(frame, cycles);

        break;
    }

    case SYSCALL_ID_UND: {
        print_exception_infos(EXC_SVC, frame);
        scheduler_on_thread_exit(frame);
        break;
    }

    default:
        print_exception_infos(EXC_SVC, frame);
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
    
    uart_irq_service_rx();
    if (!is_ring_empty()){
        char c = ring_peek();

        if (c == 'N' || c == 'P' || c == 'C' || c == 'U' || c == 'X') {
            uart_getc();  /* consume the character */
            volatile char dummy = 0;
                
            switch (c) {
                case 'N':
                    dummy = *((volatile char *)NULL);
                    break;

                case 'P':
                    ((void (*)(void))NULL)();
                    break;    
                    
                case 'C':
                    *((volatile char *)&ld_section_kernel_text) = dummy;
                    break;
                        
                case 'U':
                    dummy = *((volatile char *)0xFFFFFFFFu);
                    break;

                case 'X':
                    ((void (*)(void))&ld_section_user_text)();
                    break;
            }
            (void)dummy;
        }
    } 
    while (!is_io_queue_empty() && !is_ring_empty()) {
        scheduler_wake_blocked_on_io(uart_getc());
    }

    if (pending1 & IRQCTRL_TIMER_C1_BIT) {
        clear_timer_interrupt();
        set_next_timer_interrupt();
        scheduler_update_sleep_q();
        scheduler_on_timer(frame);
    }
}