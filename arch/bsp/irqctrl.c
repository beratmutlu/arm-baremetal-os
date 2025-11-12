#include <stdint.h>
#include <stdbool.h>
#include <arch/bsp/irqctrl.h>


void irqctrl_enable_timer(void) {
    *IRQCTRL_ENABLE_TIMER_C1 = (1 << 1);
}

void irqctrl_enable_uart(void) {
    *IRQCTRL_ENABLE_PL011    = (1 << 25);
}

uint32_t irqctrl_pending1(void) {
    volatile uint32_t *pending1 = (volatile uint32_t*)IRQ_PENDING_1;
    return *pending1;
}