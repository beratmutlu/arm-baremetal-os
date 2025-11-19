#include "kernel/exc_frame_layout.h"
#include "arch/cpu/cpu.h"

.syntax unified
.arch armv7-a
.arm
.text

.global arm_init_stacks
.type arm_init_stacks, %function

.extern svc_stack
.extern irq_stack
.extern abt_stack
.extern und_stack
.extern sys_stack

arm_init_stacks:
    mrs r0, cpsr
    orr r0, r0, #CPSR_IRQ_FIQ_DISABLE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_SVC
    msr cpsr_c, r1
    ldr sp, =svc_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_IRQ
    msr cpsr_c, r1
    ldr sp, =irq_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_ABT
    msr cpsr_c, r1
    ldr sp, =abt_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_UND
    msr cpsr_c, r1
    ldr sp, =und_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_SYS
    msr cpsr_c, r1
    ldr sp, =sys_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bic r1, r0, #CPSR_MODE_MASK
    orr r1, r1, #CPU_SVC
    msr cpsr_c, r1
    ldr sp, =svc_stack
    add sp, sp, #EXC_STACK_SIZE
    
    bx lr

.size arm_init_stacks, . - arm_init_stacks