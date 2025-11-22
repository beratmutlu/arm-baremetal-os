#include <stdint.h>
#include <kernel/exception_print.h>
#include <kernel/exceptions.h>
#include <kernel/exc_frame_layout.h>
#include <kernel/kprintf.h>
#include <arch/cpu/banked_regs.h>
#include <arch/cpu/cpu.h>

const char* get_fsr_description(unsigned int fsr){
    static const char *fsr_sources[] = {
        [0b00000] =  "No function, reset value",
        [0b00001] =  "Alignment fault",
        [0b00010] =  "Debug event fault",
        [0b00011] =  "Access Flag fault on Section",
        [0b00100] =  "Cache maintenance operation fault",
        [0b00101] =  "Translation fault on Section",
        [0b00110] =  "Access Flag fault on Page",
        [0b00111] =  "Translation fault on Page",
        [0b01000] =  "Precise External Abort",
        [0b01001] =  "Domain fault on Section",
        [0b01011] =  "Domain fault on Page",
        [0b01100] =  "External abort on Section",
        [0b01101] =  "Permission fault on Section",
        [0b01110] =  "External abort on Page",
        [0b01111] =  "Permission fault on Page",
        [0b10000] =  "TLB conflict abort",
        [0b10100] =  "Implementation definedf fault",
        [0b10110] =  "External Abort",
        [0b11000] =  "Asynchronous parity error on memory access",
        [0b11001] =  "Synchronous parity error on memory access",
        [0b11010] =  "Implementation defined fault",
        [0b11100] =  "Synchronous parity error on translation table walk on section",
        [0b11110] =  "Synchronous parity error on translation table walk on page",
    };

    const int fsr_status_4_index = 10;
    unsigned int fsr_status = (fsr & 0b1111) |
     ((fsr & (1 << (fsr_status_4_index - 1))) >> (fsr_status_4_index - 4 - 1));

    if(fsr_status > sizeof(fsr_sources) / sizeof(const char*) ||
       fsr_sources[fsr_status] == NULL){
        return "Invalid fault status register value";
    }

    return fsr_sources[fsr_status];
}

static void print_mode_regs(const char *name, uint32_t mode, 
                           bool is_current_mode,
                           const struct exc_frame *frame) {
    uint32_t lr, sp, spsr;
    
    if (is_current_mode) {
        lr = frame->lr;
        sp = exc_frame_get_sp(frame);
        spsr = exc_frame_get_spsr(frame);
    } else {
        lr = cpu_get_banked_lr(mode);
        sp = cpu_get_banked_sp(mode);
        spsr = cpu_get_banked_spsr(mode);
    }
    
    kprintf("\n%s| LR: 0x%08x | SP: 0x%08x | SPSR: ", name, lr, sp);
    cpu_print_psr(spsr);
}
void print_exception_infos(enum exc_kind kind, const struct exc_frame* frame) {
    const char *name =
        (kind == EXC_UND)  ? "Undefined Instruction" :
        (kind == EXC_SVC)  ? "Supervisor Call" :
        (kind == EXC_PABT) ? "Prefetch Abort" :
        (kind == EXC_DABT) ? "Data Abort" :
        (kind == EXC_IRQ)  ? "IRQ" : "INVALID";

    kprintf("############ EXCEPTION ############\n");
    kprintf("%s an Adresse: 0x%08x\n", name, frame->lr);
    
    if (kind == EXC_DABT) {
        uint32_t dfsr = mmu_get_dfsr();
        uint32_t dfar = mmu_get_dfar();
        const char* desc = get_fsr_description(dfsr);
        kprintf("Data Fault Status Register: 0x%08x -> %s\n", dfsr, desc);
        kprintf("Data Fault Adress Register: 0x%08x\n", dfar);
    }
    
    if (kind == EXC_PABT) {
        uint32_t ifsr = mmu_get_ifsr();
        uint32_t ifar = mmu_get_ifar();
        const char* desc = get_fsr_description(ifsr);
        kprintf("Instruction Fault Status Register: 0x%08x -> %s\n", ifsr, desc);
        kprintf("Instruction Fault Adress Register: 0x%08x\n", ifar);
    }
    
    kprintf("\n>> Registerschnappschuss <<\n");
    kprintf("R0: 0x%08x  R5: 0x%08x  R10: 0x%08x\n", frame->r[0], frame->r[5], frame->r[10]);
    kprintf("R1: 0x%08x  R6: 0x%08x  R11: 0x%08x\n", frame->r[1], frame->r[6], frame->r[11]);
    kprintf("R2: 0x%08x  R7: 0x%08x  R12: 0x%08x\n", frame->r[2], frame->r[7], frame->r[12]);
    kprintf("R3: 0x%08x  R8: 0x%08x\n", frame->r[3], frame->r[8]);
    kprintf("R4: 0x%08x  R9: 0x%08x\n", frame->r[4], frame->r[9]);
    
    kprintf("\n>> Modusspezifische Register <<\n");
    
    kprintf("User/System | LR: 0x%08x | SP: 0x%08x | CPSR: ",
            cpu_get_banked_lr(CPU_USR), cpu_get_banked_sp(CPU_USR));
    cpu_print_psr(cpu_get_cpsr());
    
    print_mode_regs("IRQ         ", CPU_IRQ, kind == EXC_IRQ, frame);
    print_mode_regs("Abort       ", CPU_ABT, kind == EXC_PABT || kind == EXC_DABT, frame);
    print_mode_regs("Undefined   ", CPU_UND, kind == EXC_UND, frame);
    print_mode_regs("Supervisor  ", CPU_SVC, kind == EXC_SVC, frame);
    
    
    kprintf("\n");
}
