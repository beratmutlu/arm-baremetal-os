
#include <stddef.h>
#include <stdint.h>
#include <kernel/exc_frame_layout.h>

__attribute__((aligned(8))) unsigned char svc_stack[EXC_STACK_SIZE];
__attribute__((aligned(8))) unsigned char irq_stack[EXC_STACK_SIZE];
__attribute__((aligned(8))) unsigned char abt_stack[EXC_STACK_SIZE];
__attribute__((aligned(8))) unsigned char und_stack[EXC_STACK_SIZE];
__attribute__((aligned(8))) unsigned char sys_stack[EXC_STACK_SIZE];


