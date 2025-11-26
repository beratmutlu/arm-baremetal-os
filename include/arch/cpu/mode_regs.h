/**
 * @file mode_regs.h
 * @brief CPU mode register and MMU fault register access.
 *
 * Provides a unified API to access LR, SP, SPSR, and CPSR for any ARM CPU mode,
 * plus selected MMU fault status/address registers.
 *
 * For modes with banked registers (IRQ, SVC, ABT, UND), values are read from
 * the corresponding banked registers. For the currently active mode, the
 * helpers fall back to the unbanked SP/LR/SPSR instructions where appropriate.
 *
 * Typical usage in exception handlers:
 *  - Use exc_frame_* accessors for the exception mode itself.
 *  - Use cpu_get_banked_*() to inspect other modes without switching into them.
 *
 * This interface abstracts over:
 *   - banked registers (SP_irq, LR_svc, SPSR_abt, …)
 *   - unbanked registers (CPSR, MMU fault registers)
 */

#ifndef MODE_REGS_H
#define MODE_REGS_H

#include <stdint.h>

/**
 * @brief Read the stack pointer of a specific CPU mode.
 * @param mode CPU mode (e.g., CPU_USR, CPU_IRQ, CPU_SVC)
 * @return Stack pointer value
 */
uint32_t cpu_get_banked_sp(uint32_t mode);

/**
 * @brief Read the link register of a specific CPU mode.
 * @param mode CPU mode
 * @return Link register value
 */
uint32_t cpu_get_banked_lr(uint32_t mode);

/**
 * @brief Read the saved program status register of a specific CPU mode.
 * @param mode CPU mode (must be a privileged mode with SPSR)
 * @return SPSR value
 */
uint32_t cpu_get_banked_spsr(uint32_t mode);

/**
 * @brief Read the current CPSR.
 * @return Current Program Status Register value
 */
uint32_t cpu_get_cpsr(void);

/**
 * @brief Read SPSR from the current exception mode.
 * @return Saved Program Status Register
 */
uint32_t cpu_get_spsr(void);

/**
 * @brief Read Data Fault Status Register.
 * @return DFSR value
 */
uint32_t mmu_get_dfsr(void);

/**
 * @brief Read Data Fault Address Register.
 * @return DFAR value
 */
uint32_t mmu_get_dfar(void);

/**
 * @brief Read Instruction Fault Status Register.
 * @return IFSR value
 */
uint32_t mmu_get_ifsr(void);

/**
 * @brief Read Instruction Fault Address Register.
 * @return IFAR value
 */
uint32_t mmu_get_ifar(void);

#endif /* MODE_REGS_H */