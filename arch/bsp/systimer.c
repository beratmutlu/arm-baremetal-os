#include <stdint.h>
#include <arch/bsp/systimer.h>
#include <config.h>
#include <arch/bsp/bcm2835_systimer_regs.h>


void systimer_init(void) {
    SYSTIMER->CS = (1 << 1);
    uint32_t current = SYSTIMER->CLO;
    SYSTIMER->C1 = current + TIMER_INTERVAL;
}

void clear_timer_interrupt(void) {
    SYSTIMER->CS = (1 << 1);
}

void set_next_timer_interrupt(void) {
    uint32_t current = SYSTIMER->CLO;
    SYSTIMER->C1 = current + TIMER_INTERVAL;
}