
/**
 * @file kprintf.h
 * @brief Kernel printf wrapper that writes to the serial console (UART).
 *
 * Thin convenience function over kvprintf: formats a message and emits bytes
 * via the UART backend.
 * @ingroup kprintf_api
 */

#ifndef KPRINTF_H
#define KPRINTF_H

/**
 * @brief Print a formatted message to the serial console.
 *
 * Contract:
 *  - Blocks while the UART TX FIFO is full (busy-wait).
 *  - Writes the formatted output using the UART backend.
 *  - Supported conversions are those provided by @ref kvprintf.
 *  - Returns no status.
 *
 * @param fmt Format string (must not be NULL).
 * @param ... Arguments according to @p fmt.
 */
void kprintf(const char *fmt, ...);

#endif /* KPRINTF_H */
