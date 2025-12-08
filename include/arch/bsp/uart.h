
/**
 * @file uart.h
 * @brief Public UART API built on the PL011 BSP.
 * 
 * Simple, synchronous convenience functions.
 * 
 */

/** @defgroup uart_api UART API (public)
 *  @brief Blocking UART functions (`uart_init`, `uart_putc`, ...).
 *  @see @ref uart_bsp for low-level register details.
 *  @{
 */

#ifndef UART_H
#define UART_H

#include <stdbool.h>

/** 
 * @brief Initialize UART hardware.
 * Clears error status.
 */
void uart_init(void);


bool is_ring_empty();

/**
 * @brief Blocking read of one byte.
 * @return The received character.
 */
char uart_getc(void);

/** @brief Blocking write of one byte. */
void uart_putc(char c);

/** @brief Blocking write of a \\0-terminated string. */
void uart_puts(const char *s);

void uart_irq_enable(void);
void uart_irq_service_rx(void);
bool uart_irq_rx_pending(void);
#endif /* UART_H */
/** @} */ /* end of uart_api */