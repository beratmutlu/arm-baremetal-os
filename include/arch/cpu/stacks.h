/**
 * @file stacks.h
 * @brief ARM mode-specific stack initialization.
 * @defgroup cpu_stacks CPU Stack Management
 * @brief Stack initialization for ARM exception modes.
 * @{
 */

#ifndef STACKS_H
#define STACKS_H

#include <stdint.h>

/**
 * @brief Initialize stack pointers for all ARM exception modes.
 *
 * Configures SP for Supervisor, IRQ, Abort, Undefined, and System modes.
 * Must be called early in boot sequence before enabling exceptions.
 */
void arm_init_stacks(void);

/**
 * @brief Exception vector table base address.
 */
extern unsigned char vectors_table[];

/**
 * @brief Set the Vector Base Address Register (VBAR).
 * @param base Physical address of exception vector table
 */
void arm_set_vbar(void *base);

#endif /* STACKS_H */
/** @} */ /* end of cpu_stacks */