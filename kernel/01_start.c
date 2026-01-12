#include <stdint.h>
#include <stdbool.h>
#include <config.h>
#include <arch/cpu/stacks.h>
#include <arch/cpu/cpu.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/bcm2835_irq.h>
#include <kernel/kprintf.h>
#include <kernel/exceptions.h>
#include <kernel/exc_triggers.h>
#include <kernel/threads.h>
#include <kernel/scheduler.h>
#include <arch/cpu/mmu.h>

extern bool irq_debug;
extern void register_checker(void);

void main [[gnu::weak]] (void *args);

void start_kernel[[noreturn]](void);
void start_kernel[[noreturn]](void) {
    uart_init();
    arm_init_stacks();
    arm_set_vbar(&vectors_table);
    systimer_init();
    irqctrl_enable_uart();
    irqctrl_enable_timer();
    
    threads_init();
    scheduler_init();
    mmu_init();
    uart_puts(">>> before mmu_enable\n");

    //mmu_enable();
    uart_puts(">>> after mmu_enable\n");

    kprintf("=== Betriebssystem gestartet ===\n");
    test_kernel();
    scheduler_thread_create(main, NULL, 0);
    scheduler_start();

    __builtin_unreachable();
}