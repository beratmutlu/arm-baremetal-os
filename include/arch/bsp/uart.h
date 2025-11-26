
/**
 * @file uart.h
 * @brief Public UART API built on the PL011 BSP.
 * 
 *
 * Provides blocking send/receive functions implemented on top of an
 * interrupt-driven receive ring buffer.
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
 *
 * Clears error flags and enables RX interrupts.
 * Must be called before any other UART operations.
 */
void uart_init(void);

/**
 * @brief Blocking read of one character.
 *
 * Waits until a character is available in the receive buffer.
 * @return The received character
 */
char uart_getc(void);

/**
 * @brief Blocking write of one character.
 * @param c Character to transmit
 */
void uart_putc(char c);

/** @brief Blocking write of a \\0-terminated string. */
void uart_puts(const char *s);

/**
 * @brief Enable UART receive interrupts.
 *
 * Configures the PL011 to generate interrupts when data is received.
 */
void uart_irq_enable(void);

/**
 * @brief Service pending UART receive interrupt.
 *
 * Reads all available characters from the FIFO into the ring buffer
 * and clears the interrupt flag. Must be called from IRQ handler.
 */
void uart_irq_service_rx(void);

/**
 * @brief Check if UART receive interrupt is pending.
 * @return true if RX interrupt is pending, false otherwise
 */
bool uart_irq_rx_pending(void);
#endif /* UART_H */
/** @} */ /* end of uart_api */