/**
 * @file exception_print.h
 * @brief Formatted exception information output.
 *
 * Provides a unified function to print exception details including:
 * - Exception type and faulting address
 * - Fault status registers (for aborts)
 * - Register snapshot
 * - Mode-specific banked registers
 */

#ifndef EXCEPTION_PRINT_H
#define EXCEPTION_PRINT_H

#include <kernel/exceptions.h>

struct exc_frame;

/**
 * @brief Print detailed information about an exception.
 * @param kind Type of exception
 * @param frame Pointer to saved register context
 *
 * Outputs a formatted dump including register state, fault registers
 * (if applicable), and all mode-specific banked registers.
 */
void print_exception_infos(enum exc_kind kind, const struct exc_frame* frame);

#endif /* EXCEPTION_PRINT_H */