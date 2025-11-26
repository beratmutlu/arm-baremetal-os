/**
 * @file bcm2835_irq.h
 * @brief BCM2835 interrupt controller register definitions.
 * @defgroup irq_bsp BCM2835 IRQ Controller BSP
 * @brief Low-level IRQ controller register access and enable helpers.
 * @{
 */

#ifndef BCM2835_IRQ_H
#define BCM2835_IRQ_H

#include <stdint.h>
#include <stddef.h>
#include <arch/bsp/bcm2835_base.h>

/** @brief Bit mask for System Timer Compare 1 interrupt. */
#define IRQCTRL_TIMER_C1_BIT (1u << 1)

/** @brief Bit mask for PL011 UART interrupt. */
#define IRQCTRL_PL011_BIT (1u << 25)
/**
 * @struct bcm2835_irqctrl_regs
 * @brief Memory-mapped IRQ register block.
 *
 * Use @ref BCM2835_IRQCTRL to access the device instance.
 */
struct bcm2835_irqctrl_regs {
    volatile uint32_t unused0;      /* 0x00 */
    volatile uint32_t pending_1;    /* 0x04 */
    volatile uint32_t pending_2;    /* 0x08 */
    volatile uint32_t unused1;      /* 0x0C */
    volatile uint32_t enable_1;     /* 0x10 */
    volatile uint32_t enable_2;     /* 0x14 */
};

_Static_assert(offsetof(struct bcm2835_irqctrl_regs, pending_1)  == 0x04, "bcm2835_irqctrl_regs: pending_1 OFFSET");
_Static_assert(offsetof(struct bcm2835_irqctrl_regs, pending_2)  == 0x08, "bcm2835_irqctrl_regs: pending_2 OFFSET");
_Static_assert(offsetof(struct bcm2835_irqctrl_regs, enable_1)   == 0x10, "bcm2835_irqctrl_regs: enable_1 OFFSET");
_Static_assert(offsetof(struct bcm2835_irqctrl_regs, enable_2)   == 0x14, "bcm2835_irqctrl_regs: enable_2 OFFSET");
_Static_assert(sizeof(struct bcm2835_irqctrl_regs)               == 0x18, "bcm2835_irqctrl_regs: struct size should reach 0x18");

#define BCM2835_IRQCTRL ((volatile struct bcm2835_irqctrl_regs *)(uintptr_t)BCM2835_IRQCTRL_BASE_PHYS)


#endif /* BCM2835_IRQ_H */
/** @} */ /* end of irq_bsp */