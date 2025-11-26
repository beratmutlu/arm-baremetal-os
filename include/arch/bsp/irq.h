
/**
 * @file irq.h
 * @brief High-level BCM2835 interrupt controller API.
 *
 * Provides functions to enable interrupts and query interrupt sources
 * via the BCM2835 IRQ controller.
 */
#ifndef IRQ_H
#define IRQ_H

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

/**
 * @brief Read pending interrupt status for bank 2.
 * @return Bitmask of pending interrupts
 */
uint32_t irqctrl_pending2(void);

/**
 * @brief Check if PL011 UART interrupt is pending.
 * @return true if UART interrupt is pending, false otherwise
 */
bool irqctrl_is_PL011(void);

/**
 * @brief Check if System Timer interrupt is pending.
 * @return true if timer interrupt is pending, false otherwise
 */
bool irqctrl_is_timer(void);
#endif /* IRQ_H */
