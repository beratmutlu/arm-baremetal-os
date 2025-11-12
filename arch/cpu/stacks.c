// arch/cpu/stacks.c
#include <stddef.h>
#include <stdint.h>

__attribute__((aligned(8))) unsigned char svc_stack[0x1000];
__attribute__((aligned(8))) unsigned char irq_stack[0x1000];
__attribute__((aligned(8))) unsigned char abt_stack[0x1000];
__attribute__((aligned(8))) unsigned char und_stack[0x1000];
__attribute__((aligned(8))) unsigned char sys_stack[0x1000];


