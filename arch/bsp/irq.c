#include <stdint.h>
#include <arch/bsp/bcm2835_irq.h>


uint32_t irqctrl_pending1(void) {
    return BCM2835_IRQCTRL->pending_1;
}

void irqctrl_enable_timer(void){
    BCM2835_IRQCTRL->enable_1 |= IRQCTRL_TIMER_C1_BIT;
}

void irqctrl_enable_uart(void) {
    BCM2835_IRQCTRL->enable_2 |= IRQCTRL_PL011_BIT;
}

uint32_t irqctrl_pending2(void) {
    return BCM2835_IRQCTRL->pending_2;
}
