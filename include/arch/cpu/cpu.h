#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#define CPSR_MODE_MASK 0x1F

enum cpu_mode {
    CPU_USR = 0x10,
    CPU_FIQ = 0x11,
    CPU_IRQ = 0x12,
    CPU_SVC = 0x13,
    CPU_ABT = 0x17,
    CPU_UND = 0x1B,
    CPU_SYS = 0x1F,
};

enum cpsr_bits {
    CPSR_N = 1u << 31,
    CPSR_Z = 1u << 30,
    CPSR_C = 1u << 29,
    CPSR_V = 1u << 28,
    CPSR_E = 1u << 9,
    CPSR_I = 1u << 7,
    CPSR_F = 1u << 6,
    CPSR_T = 1u << 5,
};

void cpu_print_psr(uint32_t psr);

static inline void cpu_irq_disable(void) {
    asm volatile ("cpsid i");
}

static inline void cpu_irq_enable(void) {
    asm volatile ("cpsie i");
}

static inline void do_data_abort(void) {
    volatile uint32_t *invalid_addr = (uint32_t *)0xBADBADADu;
    *invalid_addr = 0xBADBADADu;
}

static inline void do_prefetch_abort(void) {
    void (*bad_func)(void) = (void (*)(void))0xBADBADF0u;
    bad_func();
}

static inline void do_supervisor_call(void) {
    asm volatile("svc #0");
}

static inline void do_undefined_inst(void) {
    asm volatile(".word 0xE7F000F0");
}

#endif