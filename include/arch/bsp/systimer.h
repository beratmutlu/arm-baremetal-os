
/**
 * @file systimer.h
 * @brief BCM2835 System Timer driver API.
 * @defgroup systimer_api System Timer API
 * @brief High-level System Timer control functions built on the Systimer BSP.
 * @{
 */
#ifndef SYSTIMER_H
#define SYSTIMER_H

/**
 * @brief Initialize System Timer for periodic interrupts.
 *
 * Clears any pending interrupt and schedules the first interrupt
 * based on TIMER_INTERVAL from config.h.
 */
void systimer_init(void);

/**
 * @brief Clear pending System Timer Compare 1 interrupt.
 *
 * Must be called in IRQ handler to acknowledge the interrupt.
 */
void clear_timer_interrupt(void);

/**
 * @brief Schedule next timer interrupt.
 *
 * Sets Compare 1 register to trigger after TIMER_INTERVAL microseconds
 * from current counter value.
 */
void set_next_timer_interrupt(void);

#endif /* SYSTIMER_H */
/** @} */ /* end of systimer_api */