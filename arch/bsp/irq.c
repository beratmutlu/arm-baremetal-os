/**
 * @file irq.c
 * @brief BCM2835 interrupt controller driver implementation.
 *
 * Provides high-level functions to enable interrupt sources and
 * query pending interrupt status from the BCM2835 IRQ controller.
 */

#include <stdint.h>
#include <stdbool.h>
#include <arch/bsp/bcm2835_irq.h>
#include <arch/bsp/irq.h>

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

bool irqctrl_is_PL011(void) {
    return BCM2835_IRQCTRL->pending_2 & IRQCTRL_PL011_BIT;
}

bool irqctrl_is_timer(void) {
    return BCM2835_IRQCTRL->pending_1 & IRQCTRL_TIMER_C1_BIT;
}