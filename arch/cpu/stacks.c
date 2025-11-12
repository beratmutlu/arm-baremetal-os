#include <arch/cpu/stacks.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((aligned(8))) static unsigned char svc_stack[0x1000];
__attribute__((aligned(8))) static unsigned char irq_stack[0x1000];
__attribute__((aligned(8))) static unsigned char abt_stack[0x1000];
__attribute__((aligned(8))) static unsigned char und_stack[0x1000];
__attribute__((aligned(8))) static unsigned char sys_stack[0x1000];


asm(".globl __stack_svc_end\n__stack_svc_end = svc_stack + 0x1000");
asm(".globl __stack_irq_end\n__stack_irq_end = irq_stack + 0x1000");
asm(".globl __stack_abt_end\n__stack_abt_end = abt_stack + 0x1000");
asm(".globl __stack_und_end\n__stack_und_end = und_stack + 0x1000");
asm(".globl __stack_sys_end\n__stack_sys_end = sys_stack + 0x1000");
