/**
 * @file systimer.c
 * @brief BCM2835 System Timer driver implementation.
 *
 * Configures Compare 1 register for periodic interrupts at intervals
 * defined by TIMER_INTERVAL. The 1MHz counter provides microsecond
 * resolution timing.
 */

#include <stdint.h>
#include <arch/bsp/systimer.h>
#include <config.h>
#include <arch/bsp/bcm2835_systimer_regs.h>


void clear_timer_interrupt(void) {
    SYSTIMER->CS = SYSTIMER_M1;
}

void set_next_timer_interrupt(void) {
    uint32_t current = SYSTIMER->CLO;
    SYSTIMER->C1 = current + TIMER_INTERVAL;
}

void systimer_init(void) {
    clear_timer_interrupt();
    set_next_timer_interrupt();
}