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
    orr r0, r0, #0xC0
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
    ldr sp, =svc_stack
    add sp, sp, #0x1000
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c, r1
    ldr sp, =irq_stack
    add sp, sp, #0x1000
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c, r1
    ldr sp, =abt_stack
    add sp, sp, #0x1000
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c, r1
    ldr sp, =und_stack
    add sp, sp, #0x1000
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x1F
    msr cpsr_c, r1
    ldr sp, =sys_stack
    add sp, sp, #0x1000
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
    ldr sp, =svc_stack
    add sp, sp, #0x1000
    
    bx lr

.size arm_init_stacks, . - arm_init_stacks