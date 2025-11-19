/**
 * @file bcm2835_irq.h
 * @brief BCM2835 interrupt controller register definitions and control functions.
 * @defgroup irq_bsp BCM2835 IRQ Controller BSP
 * @brief Low-level IRQ controller register access and enable helpers.
 * @{
 */

#ifndef BCM2835_IRQ_H
#define BCM2835_IRQ_H

#include <stdint.h>
#include <arch/bsp/bcm2835_base.h>

/** @brief Bit mask for System Timer Compare 1 in pending register. */
#define IRQCTRL_PENDING_TIMER_C1_BIT (1u << 1)

struct bcm2835_irqctrl_regs {
    volatile uint32_t unused1;          /* 0x00 */
    volatile uint32_t pending_1;        /* 0x04 */
    volatile uint32_t unused2[2];       /* 0x08-0x0C*/
    volatile uint32_t enable_1;         /* 0x10 */
};

#define BCM2835_IRQCTRL ((volatile struct bcm2835_irqctrl_regs *)(uintptr_t)BCM2835_IRQCTRL_BASE_PHYS)

/** @brief Bit mask for System Timer Compare 1 interrupt. */
#define IRQCTRL_ENABLE_TIMER_C1_BIT  (1u << 1)

/** @brief Bit mask for PL011 UART interrupt. */
#define IRQCTRL_ENABLE_PL011_BIT     (1u << 25)

/**
 * @brief Enable System Timer Compare 1 interrupt.
 */
void irqctrl_enable_timer(void);

/**
 * @brief Enable PL011 UART interrupt.
 */
void irqctrl_enable_uart(void);

/**
 * @brief Read pending interrupt status for bank 1.
 * @return Bitmask of pending interrupts
 */
uint32_t irqctrl_pending1(void);

#endif /* BCM2835_IRQ_H */
/** @} */ /* end of irq_bsp */