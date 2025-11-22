#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <stdint.h>
uint32_t decode_svc_imm(const uint32_t lr);

#endif