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
#include <arch/cpu/mmu.h>
#include <user/main.h>
#include <user/syscalls.h>
#include <stdarg.h>
#include <lib/kvprintf.h>

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

        if (!f || arg_size > 4000) {
            scheduler_on_thread_exit(frame);
            break;
        }
        thread_t *curr = scheduler_curr();
        if (mmu_as_get_refcount(curr->asid) >= 4) {
            break;  /* AS thread limit reached */
        }
        mmu_as_addref(curr->asid);
        scheduler_thread_create_in_as(f, args, arg_size, curr->asid);
        scheduler_yield(frame);  /* Let new thread run before parent continues */
        break;
    }

    case SYSCALL_ID_CREATE_PROCESS: {
        void (*f)(void *) = (void (*)(void *))(uintptr_t)frame->r[0];
        void *args        = (void *)(uintptr_t)frame->r[1];
        unsigned arg_size = (unsigned)frame->r[2];

        if (!f || arg_size > 4000) {
            scheduler_on_thread_exit(frame);
            break;
        }
        uint32_t new_asid = mmu_as_create();
        if (new_asid == AS_INVALID) {
            break;  /* Out of address spaces */
        }
        scheduler_thread_create_in_as(f, args, arg_size, new_asid);
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

    case SYSCALL_ID_VPRINTF: {
        const char *fmt = (const char *)frame->r[0];
        va_list *ap = (va_list *)frame->r[1];
        kvprintf(uart_putc_adapter, NULL, fmt, *ap);
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

    bool woke_up = false; 
    while (!is_io_queue_empty() && !is_ring_empty()) {
        scheduler_wake_blocked_on_io(uart_getc());
        woke_up = true;
    }

  bool timer_tick = (pending1 & IRQCTRL_TIMER_C1_BIT) != 0;

  if (timer_tick) {
    clear_timer_interrupt();
    set_next_timer_interrupt();
    scheduler_update_sleep_q();
  }

  if (timer_tick || (woke_up && scheduler_curr()->is_idle)) {
    scheduler_on_timer(frame);
  }
}