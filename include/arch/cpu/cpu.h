/**
 * @file cpu.h
 * @brief ARM CPU control and mode management.
 *
 * Provides functions for interrupt control, mode queries, and
 * program status register formatting.
 */

#ifndef CPU_H
#define CPU_H

#define CPSR_MODE_MASK 0x1F
#define CPSR_IRQ_FIQ_DISABLE 0xC0

/**
 * @brief ARM processor modes.
 */
#define CPU_USR     0x10
#define CPU_FIQ     0x11
#define CPU_IRQ     0x12
#define CPU_SVC     0x13
#define CPU_ABT     0x17
#define CPU_UND     0x1B
#define CPU_SYS     0x1F


#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief CPSR flag bits.
 */
enum cpsr_bits {
    CPSR_N = 1u << 31,  /**< Negative flag */
    CPSR_Z = 1u << 30,  /**< Zero flag */
    CPSR_C = 1u << 29,  /**< Carry flag */
    CPSR_V = 1u << 28,  /**< Overflow flag */
    CPSR_E = 1u << 9,   /**< Endianness */
    CPSR_I = 1u << 7,   /**< IRQ disable */
    CPSR_F = 1u << 6,   /**< FIQ disable */
    CPSR_T = 1u << 5,   /**< Thumb state */
};

/**
 * @brief Print formatted PSR (CPSR or SPSR).
 * @param psr Program Status Register value
 *
 * Outputs flags, mode, and hex value in a human-readable format.
 */
void cpu_print_psr(uint32_t psr);

/**
 * @brief Disable IRQ interrupts (sets I bit in CPSR).
 */
static inline void cpu_irq_disable(void) {
    asm volatile("cpsid i" ::: "memory");
}

/**
 * @brief Enable IRQ interrupts (clears I bit in CPSR).
 */
static inline void cpu_irq_enable(void) {
    asm volatile("cpsie i" ::: "memory");
}

bool is_user_mode(const uint32_t cpsr);

#endif /* !__ASSEMBLER__ */

#endif /* CPU_H */