/**
 * @file exceptions.h
 *
 * Declares C-level exception handlers called from assembly trampolines.
 * Each handler receives a pointer to the saved register frame and performs
 * exception-specific processing (logging, cleanup, or panic).
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdbool.h>

/**
 * @defgroup exc_handlers Exception handlers
 * @brief High-level C handlers for ARM exceptions.
 *
 * Each handler receives a pointer to the saved exception frame and either
 * halts the system (for synchronous exceptions) or services pending IRQs.
 * @{
 */

/**
 * @brief Exception types recognized by the kernel.
 */
enum exc_kind {
    EXC_UND,   /**< Undefined Instruction */
    EXC_SVC,   /**< Supervisor Call */
    EXC_PABT,  /**< Prefetch Abort */
    EXC_DABT,  /**< Data Abort */
    EXC_IRQ    /**< Hardware Interrupt Request */
};

/* Forward declaration */
struct exc_frame;

/**
 * @brief Undefined instruction exception handler.
 * @param frame Pointer to saved register context
 */
void und_handler_c(struct exc_frame* frame);

/**
 * @brief Supervisor call exception handler.
 * @param frame Pointer to saved register context
 */
void svc_handler_c(struct exc_frame* frame);

/**
 * @brief Prefetch abort exception handler.
 * @param frame Pointer to saved register context
 */
void pabt_handler_c(struct exc_frame* frame);

/**
 * @brief Data abort exception handler.
 * @param frame Pointer to saved register context
 */
void dabt_handler_c(struct exc_frame* frame);

/**
 * @brief Hardware interrupt (IRQ) handler.
 * @param frame Pointer to saved register context
 *
 * This handler is returnable—execution resumes after servicing IRQ.
 */
void irq_handler_c(struct exc_frame* frame);

#endif /* EXCEPTIONS_H */
/** @} */ /* end of exc_handlers */
