/**
 * @file exc_triggers.h
 * @brief Test functions to deliberately trigger exceptions.
 *
 * These functions are used for testing exception handlers by
 * intentionally causing undefined instructions, data aborts, etc.
 */

#ifndef EXC_TRIGGERS_H
#define EXC_TRIGGERS_H


/**
 * @brief Trigger a data abort by accessing misaligned memory.
 */
static inline void do_data_abort(void) {
    volatile uint32_t *invalid_addr = (uint32_t *)0xBADBADADu;
    *invalid_addr = 0xBADBADADu;
}

/**
 * @brief Trigger a prefetch abort by branching to invalid address.
 */
static inline void do_prefetch_abort(void) {
    asm volatile("bkpt #0");
}

/**
 * @brief Trigger a supervisor call exception.
 */
static inline void do_supervisor_call(void) {
    asm volatile(
        "mrs r0, cpsr        \n"
        "bic r0, r0, #0x1f   \n"  
        "orr r0, r0, #0x1f   \n"  
        "msr cpsr_c, r0      \n"  
        "svc #0              \n"  
    );
}

/**
 * @brief Trigger an undefined instruction exception.
 */
static inline void do_undefined_inst(void) {
    asm volatile(".word 0xE7F000F0");
}

#endif /* EXC_TRIGGERS_H */