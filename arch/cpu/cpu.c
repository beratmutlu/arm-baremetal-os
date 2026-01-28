#include <arch/cpu/cpu.h>
#include <kernel/kprintf.h>

static const char* cpu_mode_str(uint32_t mode) {
    switch (mode) {
        case CPU_USR: return "       User"; 
        case CPU_FIQ: return "        FIQ"; 
        case CPU_IRQ: return "        IRQ";
        case CPU_SVC: return " Supervisor"; 
        case CPU_ABT: return "      Abort"; 
        case CPU_UND: return "  Undefined";
        case CPU_SYS: return "     System"; 
        default:      return "    Invalid"; 
    }
}
void cpu_print_psr(uint32_t psr) {
    char n = (psr & CPSR_N) ? 'N' : '_';
    char z = (psr & CPSR_Z) ? 'Z' : '_';
    char c = (psr & CPSR_C) ? 'C' : '_';
    char v = (psr & CPSR_V) ? 'V' : '_';
    char e = (psr & CPSR_E) ? 'E' : '_';
    char i = (psr & CPSR_I) ? 'I' : '_';
    char f = (psr & CPSR_F) ? 'F' : '_';
    char t = (psr & CPSR_T) ? 'T' : '_';
    
    const char* mode = cpu_mode_str(psr & CPSR_MODE_MASK);
    
    kprintf("%c%c%c%c %c %c%c%c", n, z, c, v, e, i, f, t);
    
    kprintf(" %s", mode);
    kprintf(" 0x%08x", (unsigned int) psr);   
}

bool is_user_mode(const uint32_t cpsr) {
    return (cpsr & CPSR_MODE_MASK) == CPU_USR;
}