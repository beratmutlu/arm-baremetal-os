/**
 * @file banked_regs.h
 * @brief Access to ARM banked registers across CPU modes.
 * @defgroup banked_regs Banked Register Access
 * @brief APIs for reading banked SP, LR, SPSR, and fault registers.
 * @{
 *
 * Provides a unified API to read banked SP, LR, and SPSR from
 * different CPU modes without direct mode switching.
 */

#ifndef BANKED_REGS_H
#define BANKED_REGS_H

#include <stdint.h>

/**
 * @brief Read the stack pointer of a specific CPU mode.
 * @param mode CPU mode (e.g., CPU_USR, CPU_IRQ, CPU_SVC)
 * @return Stack pointer value
 */
uint32_t cpu_get_banked_sp(uint32_t mode);


void cpu_set_banked_sp(uint32_t mode, uint32_t sp);


/**
 * @brief Read the link register of a specific CPU mode.
 * @param mode CPU mode
 * @return Link register value
 */
uint32_t cpu_get_banked_lr(uint32_t mode);


void cpu_set_banked_lr(uint32_t mode, uint32_t lr);


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

#endif /* BANKED_REGS_H */
/** @} */ /* end of banked_regs */