#include <kernel/syscall.h>
#include <stdint.h>
uint32_t decode_svc_imm(const uint32_t lr)
{
    uint32_t svc_pc = lr;

    uint32_t instr = *(uint32_t *)svc_pc;

    return instr & 0xFF;
}
