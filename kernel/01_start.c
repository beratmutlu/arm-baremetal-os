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

extern bool irq_debug;
extern void test_kernel(void);
extern void register_checker(void);

static void subprogram[[noreturn]](void);


void start_kernel[[noreturn]](void);
void start_kernel(void) {
    uart_init();
    arm_init_stacks();
    arm_set_vbar(&vectors_table);
	uart_irq_enable();
    systimer_init();
    irqctrl_enable_uart();
    irqctrl_enable_timer();
    
    cpu_irq_enable();
    
    kprintf("=== Betriebssystem gestartet ===\n");
    test_kernel();
    
    while(true) {
        char c = uart_getc();
        
        switch(c) {
            case 'd':
                irq_debug = !irq_debug;
                break;
            case 'a':
                do_data_abort();
                break;
            case 'p':
                do_prefetch_abort();
                break;
            case 's':
                do_supervisor_call();
                break;
            case 'u':
                do_undefined_inst();
                break;
            case 'c':
                register_checker();
                break;
            case 'e':
                subprogram();
                break;
            default:
                kprintf("Unknown input: [%c]\n", c);
                break;
        }
    }
}

static void subprogram(void) {
    while(true) {
        char c = uart_getc();
        for(unsigned int n = 0; n < PRINT_COUNT; n++) {
            uart_putc(c);
            volatile unsigned int i = 0;
            for(; i < BUSY_WAIT_COUNTER; i++) {}
        }
    }
}