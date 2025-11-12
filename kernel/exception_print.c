#include <kernel/exception_print.h>
#include <kernel/exceptions.h>
#include <kernel/exc_frame.h>
#include <kernel/kprintf.h>
#include <arch/cpu/cpu.h>
#include <stdint.h>

static inline unsigned fsr_status(uint32_t fsr) {
    return ((fsr >> 10) & 1u) << 4 | (fsr & 0xFu);
}

const char* fsr_description(uint32_t fsr) {
    static const char *tbl[32] = {
        [0x00] = "No function, reset value",
        [0x01] = "Alignment fault",
        [0x02] = "Debug event fault",
        [0x03] = "Access Flag fault on Section",
        [0x04] = "Cache maintenance operation fault",
        [0x05] = "Translation fault on Section",
        [0x06] = "Access Flag fault on Page",
        [0x07] = "Translation fault on Page",
        [0x08] = "Precise External Abort",
        [0x09] = "Domain fault on Section",
        [0x0B] = "Domain fault on Page",
        [0x0C] = "External abort on Section",
        [0x0D] = "Permission fault on Section",
        [0x0E] = "External abort on Page",
        [0x0F] = "Permission fault on Page",
        [0x10] = "TLB conflict abort",
        [0x14] = "Implementation-defined fault",
        [0x16] = "External Abort",
        [0x18] = "Async parity error on memory access",
        [0x19] = "Sync parity error on memory access",
        [0x1A] = "Implementation-defined fault",
        [0x1C] = "Sync parity error on TT walk (section)",
        [0x1E] = "Sync parity error on TT walk (page)",
    };
    unsigned code = fsr_status(fsr);
    return (code < 32 && tbl[code]) ? tbl[code] : "Invalid/unknown FSR";
}

void print_exception_infos(enum exc_kind kind, const struct exc_frame* frame) {
    const char *name =
        (kind == EXC_UND)  ? "Undefined" :
        (kind == EXC_SVC)  ? "Supervisor Call" :
        (kind == EXC_PABT) ? "Prefetch Abort" :
        (kind == EXC_DABT) ? "Data Abort" :
        (kind == EXC_IRQ)  ? "IRQ" : "INVALID";

    kprintf("############ EXCEPTION ############\n");
    kprintf("%s an Adresse: 0x%08x\n", name, frame->lr);
    
    if (kind == EXC_DABT) {
        uint32_t dfsr = read_dfsr();
        uint32_t dfar = read_dfar();
        const char* desc = fsr_description(dfsr);
        kprintf("Data Fault Status Register: 0x%08x -> %s\n", dfsr, desc);
        kprintf("Data Fault Adress Register: 0x%08x\n", dfar);
    }
    
    if (kind == EXC_PABT) {
        uint32_t ifsr = read_ifsr();
        uint32_t ifar = read_ifar();
        const char* desc = fsr_description(ifsr);
        kprintf("Instruction Fault Status Register: 0x%08x -> %s\n", ifsr, desc);
        kprintf("Instruction Fault Adress Register: 0x%08x\n", ifar);
    }
    
    kprintf("\n>> Registerschnappschuss <<\n");
    kprintf("R0: 0x%08x  R5: 0x%08x  R10: 0x%08x\n", 
            frame->r[0], frame->r[5], frame->r[10]);
    kprintf("R1: 0x%08x  R6: 0x%08x  R11: 0x%08x\n", 
            frame->r[1], frame->r[6], frame->r[11]);
    kprintf("R2: 0x%08x  R7: 0x%08x  R12: 0x%08x\n", 
            frame->r[2], frame->r[7], frame->r[12]);
    kprintf("R3: 0x%08x  R8: 0x%08x\n", 
            frame->r[3], frame->r[8]);
    kprintf("R4: 0x%08x  R9: 0x%08x\n", 
            frame->r[4], frame->r[9]);
    
    kprintf("\n>> Modusspezifische Register <<\n");
    
    kprintf("User/System | LR: 0x%08x | SP: 0x%08x | CPSR: ",
            read_banked_lr_usr(), read_banked_sp_usr());
    cpu_print_psr(read_cpsr());
    
    if (kind == EXC_IRQ) {
    uint32_t sp_irq_at_entry = (uint32_t)frame + sizeof(*frame);
    kprintf("\nIRQ         | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            frame->lr, sp_irq_at_entry);
    cpu_print_psr(read_spsr_current());
} else {
    kprintf("\nIRQ         | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            read_banked_lr_irq(), read_banked_sp_irq());
    cpu_print_psr(read_banked_spsr_irq());
}


if (kind == EXC_PABT || kind == EXC_DABT) {
    uint32_t sp_abt_at_entry = (uint32_t)frame + sizeof(*frame);
    kprintf("\nAbort       | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            frame->lr, sp_abt_at_entry);
    cpu_print_psr(read_spsr_current());
} else {
    kprintf("\nAbort       | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            read_banked_lr_abt(), read_banked_sp_abt());
    cpu_print_psr(read_banked_spsr_abt());
}


if (kind == EXC_UND) {
    uint32_t sp_und_at_entry = (uint32_t)frame + sizeof(*frame);
    kprintf("\nUndefined   | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            frame->lr, sp_und_at_entry);
    cpu_print_psr(read_spsr_current());
} else {
    kprintf("\nUndefined   | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            read_banked_lr_und(), read_banked_sp_und());
    cpu_print_psr(read_banked_spsr_und());
}


if (kind == EXC_SVC) {
    uint32_t sp_svc_at_entry = (uint32_t)frame + sizeof(*frame);
    kprintf("\nSupervisor  | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            frame->lr, sp_svc_at_entry);
    cpu_print_psr(read_spsr_current());
} else {
    kprintf("\nSupervisor  | LR: 0x%08x | SP: 0x%08x | SPSR: ",
            read_banked_lr_svc(), read_banked_sp_svc());
    cpu_print_psr(read_banked_spsr_svc());
}
kprintf("\n");
    
    kprintf("\n");
}