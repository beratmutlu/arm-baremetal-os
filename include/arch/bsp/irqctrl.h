#ifndef IRQCTRL_H
#define IRQCTRL_H

#include <stdint.h>

/* BCM2835 peripheral base (Raspberry Pi 2/3) */
#define PERIPH_BASE   0x3F000000u
#define IRQ_BASE      (PERIPH_BASE + 0x0000B200u)

#define IRQ_PENDING_1_BASE              (IRQ_BASE + 0x04)
#define IRQCTRL_ENABLE_TIMER_C1_BASE    (IRQ_BASE + 0x10)
#define IRQCTRL_ENABLE_PL011_BASE       (IRQ_BASE + 0x14)

/* Controller registers we touch */
#define IRQ_PENDING_1               ((volatile uint32_t *)(IRQ_PENDING_1_BASE))
#define IRQCTRL_ENABLE_TIMER_C1     ((volatile uint32_t *)(IRQCTRL_ENABLE_TIMER_C1_BASE))
#define IRQCTRL_ENABLE_PL011        ((volatile uint32_t *)(IRQCTRL_ENABLE_PL011_BASE))


/**
 * @defgroup irqctrl_regs IRQ controller register layout
 * @brief Bit positions relevant for systimer C1 and PL011.
 * @{
 */
#define IRQCTRL_ENABLE_TIMER_C1_BIT   1u   /**< System Timer Compare 1 enable bit (Timer reg). */
#define IRQCTRL_ENABLE_PL011_BIT      25u  /**< PL011 UART RX/TX enable bit (PL011 reg). */
/**@}*/

void irqctrl_enable_uart(void);

void irqctrl_enable_timer(void);


uint32_t irqctrl_pending1(void);

#endif