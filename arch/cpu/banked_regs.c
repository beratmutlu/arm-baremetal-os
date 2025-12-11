#include <arch/cpu/banked_regs.h>
#include <arch/cpu/cpu.h>
#include <kernel/panic.h>

uint32_t cpu_get_cpsr(void) {
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}

uint32_t cpu_get_spsr(void) {
    uint32_t spsr;
    asm volatile("mrs %0, spsr" : "=r"(spsr));
    return spsr;
}

uint32_t cpu_get_banked_sp(uint32_t mode) {
    uint32_t current_mode = cpu_get_cpsr() & CPSR_MODE_MASK;
    
    if (mode == current_mode) {
        uint32_t sp;
        asm volatile("mov %0, sp" : "=r"(sp));
        return sp;
    }
    
    switch (mode) {
        case CPU_USR:
        case CPU_SYS: {
            uint32_t sp;
            asm volatile("mrs %0, sp_usr" : "=r"(sp));
            return sp;
        }
        case CPU_IRQ: {
            uint32_t sp;
            asm volatile("mrs %0, sp_irq" : "=r"(sp));
            return sp;
        }
        case CPU_SVC: {
            uint32_t sp;
            asm volatile("mrs %0, sp_svc" : "=r"(sp));
            return sp;
        }
        case CPU_ABT: {
            uint32_t sp;
            asm volatile("mrs %0, sp_abt" : "=r"(sp));
            return sp;
        }
        case CPU_UND: {
            uint32_t sp;
            asm volatile("mrs %0, sp_und" : "=r"(sp));
            return sp;
        }
        default:
            panic("Invalid mode for cpu_get_banked_sp");
    }
}

void cpu_set_banked_sp(uint32_t mode, uint32_t sp) {
    uint32_t current_mode = cpu_get_cpsr() & CPSR_MODE_MASK;
    
    if (mode == current_mode) {
        asm volatile("mov sp, %0" :: "r"(sp));
        return;
    }
    
    switch (mode) {
        case CPU_USR:
        case CPU_SYS: {
            asm volatile("msr sp_usr, %0" :: "r"(sp));
            return;
        }
        case CPU_IRQ: {
            asm volatile("msr sp_irq, %0" :: "r"(sp));
            return;
        }
        case CPU_SVC: {
            asm volatile("msr sp_svc, %0" :: "r"(sp));
            return;
        }
        case CPU_ABT: {
            asm volatile("msr sp_abt, %0" :: "r"(sp));
            return;
        }
        case CPU_UND: {
            asm volatile("msr sp_und, %0" :: "r"(sp));
            return;
        }
        default:
            panic("Invalid mode for cpu_set_banked_sp");
    }
}

uint32_t cpu_get_banked_lr(uint32_t mode) {
    uint32_t current_mode = cpu_get_cpsr() & CPSR_MODE_MASK;
    
    if (mode == current_mode) {
        uint32_t lr;
        asm volatile("mov %0, lr" : "=r"(lr));
        return lr;
    }
    
    switch (mode) {
        case CPU_USR:
        case CPU_SYS: {
            uint32_t lr;
            asm volatile("mrs %0, lr_usr" : "=r"(lr));
            return lr;
        }
        case CPU_IRQ: {
            uint32_t lr;
            asm volatile("mrs %0, lr_irq" : "=r"(lr));
            return lr;
        }
        case CPU_SVC: {
            uint32_t lr;
            asm volatile("mrs %0, lr_svc" : "=r"(lr));
            return lr;
        }
        case CPU_ABT: {
            uint32_t lr;
            asm volatile("mrs %0, lr_abt" : "=r"(lr));
            return lr;
        }
        case CPU_UND: {
            uint32_t lr;
            asm volatile("mrs %0, lr_und" : "=r"(lr));
            return lr;
        }
        default:
            panic("Invalid mode for cpu_get_banked_lr");
    }
}

void cpu_set_banked_lr(uint32_t mode, uint32_t lr) {
    uint32_t current_mode = cpu_get_cpsr() & CPSR_MODE_MASK;
    
    if (mode == current_mode) {
        asm volatile("mov lr, %0" :: "r"(lr));
        return;
    }
    
    switch (mode) {
        case CPU_USR:
        case CPU_SYS: {
            asm volatile("msr lr_usr, %0" :: "r"(lr));
            return;
        }
        case CPU_IRQ: {
            asm volatile("msr lr_irq, %0" :: "r"(lr));
            return;
        }
        case CPU_SVC: {
            asm volatile("msr lr_svc, %0" :: "r"(lr));
            return;
        }
        case CPU_ABT: {
            asm volatile("msr lr_abt, %0" :: "r"(lr));
            return;
        }
        case CPU_UND: {
            asm volatile("msr lr_und, %0" :: "r"(lr));
            return;
        }
        default:
            panic("Invalid mode for cpu_set_banked_lr");
    }
}


uint32_t cpu_get_banked_spsr(uint32_t mode) {
    /* USR and SYS have no SPSR */
    if (mode == CPU_USR || mode == CPU_SYS) {
        panic("USR/SYS modes have no SPSR");
    }
    
    uint32_t current_mode = cpu_get_cpsr() & CPSR_MODE_MASK;
    
    /* Can read SPSR directly if we're in that mode */
    if (mode == current_mode) {
        return cpu_get_spsr();
    }
    
    switch (mode) {
        case CPU_IRQ: {
            uint32_t spsr;
            asm volatile("mrs %0, spsr_irq" : "=r"(spsr));
            return spsr;
        }
        case CPU_SVC: {
            uint32_t spsr;
            asm volatile("mrs %0, spsr_svc" : "=r"(spsr));
            return spsr;
        }
        case CPU_ABT: {
            uint32_t spsr;
            asm volatile("mrs %0, spsr_abt" : "=r"(spsr));
            return spsr;
        }
        case CPU_UND: {
            uint32_t spsr;
            asm volatile("mrs %0, spsr_und" : "=r"(spsr));
            return spsr;
        }
        default:
            panic("Invalid mode for cpu_get_banked_spsr");
    }
}

uint32_t mmu_get_dfsr(void) {
    uint32_t dfsr;
    asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(dfsr));
    return dfsr;
}

uint32_t mmu_get_dfar(void) {
    uint32_t dfar;
    asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(dfar));
    return dfar;
}

uint32_t mmu_get_ifsr(void) {
    uint32_t ifsr;
    asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(ifsr));
    return ifsr;
}

uint32_t mmu_get_ifar(void) {
    uint32_t ifar;
    asm volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(ifar));
    return ifar;
}