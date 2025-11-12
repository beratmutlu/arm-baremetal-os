#ifndef EXC_FRAME_H
#define EXC_FRAME_H

#include <stdint.h>

struct exc_frame {
    uint32_t r[13];
    uint32_t lr;
    uint32_t spsr;
};

static inline uint32_t read_spsr_current(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr" : "=r"(spsr));
    return spsr;
}

static inline uint32_t read_dfsr(void) {
    uint32_t dfsr;
    asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(dfsr));
    return dfsr;
}

static inline uint32_t read_dfar(void) {
    uint32_t dfar;
    asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(dfar));
    return dfar;
}

static inline uint32_t read_ifsr(void) {
    uint32_t ifsr;
    asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(ifsr));
    return ifsr;
}

static inline uint32_t read_ifar(void) {
    uint32_t ifar;
    asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(ifar));
    return ifar;
}

static inline uint32_t read_banked_sp_usr(void) {
    uint32_t sp;
    asm volatile("mrs %0, sp_usr" : "=r"(sp));
    return sp;
}

static inline uint32_t read_banked_lr_usr(void) {
    uint32_t lr;
    asm volatile("mrs %0, lr_usr" : "=r"(lr));
    return lr;
}

static inline uint32_t read_banked_sp_irq(void) {
    uint32_t sp;
    asm volatile("mrs %0, sp_irq" : "=r"(sp));
    return sp;
}

static inline uint32_t read_banked_lr_irq(void) {
    uint32_t lr;
    asm volatile("mrs %0, lr_irq" : "=r"(lr));
    return lr;
}

static inline uint32_t read_banked_spsr_irq(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr_irq" : "=r"(spsr));
    return spsr;
}

static inline uint32_t read_banked_sp_abt(void) {
    uint32_t sp;
    asm volatile("mrs %0, sp_abt" : "=r"(sp));
    return sp;
}

static inline uint32_t read_banked_lr_abt(void) {
    uint32_t lr;
    asm volatile("mrs %0, lr_abt" : "=r"(lr));
    return lr;
}

static inline uint32_t read_banked_spsr_abt(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr_abt" : "=r"(spsr));
    return spsr;
}

static inline uint32_t read_banked_sp_und(void) {
    uint32_t sp;
    asm volatile("mrs %0, sp_und" : "=r"(sp));
    return sp;
}

static inline uint32_t read_banked_lr_und(void) {
    uint32_t lr;
    asm volatile("mrs %0, lr_und" : "=r"(lr));
    return lr;
}

static inline uint32_t read_banked_spsr_und(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr_und" : "=r"(spsr));
    return spsr;
}

static inline uint32_t read_banked_sp_svc(void) {
    uint32_t sp;
    asm volatile("mrs %0, sp_svc" : "=r"(sp));
    return sp;
}

static inline uint32_t read_banked_lr_svc(void) {
    uint32_t lr;
    asm volatile("mrs %0, lr_svc" : "=r"(lr));
    return lr;
}

static inline uint32_t read_banked_spsr_svc(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr_svc" : "=r"(spsr));
    return spsr;
}

static inline uint32_t read_cpsr(void) {
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}

#endif