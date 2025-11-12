.syntax unified
.arch armv7-a
.arm
.text

.global arm_init_stacks
.type arm_init_stacks, %function

.extern __stack_svc_end
.extern __stack_irq_end
.extern __stack_abt_end
.extern __stack_und_end
.extern __stack_sys_end

arm_init_stacks:
    mrs r0, cpsr
    orr r0, r0, #0xC0
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
    ldr sp, =__stack_svc_end
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c, r1
    ldr sp, =__stack_irq_end
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c, r1
    ldr sp, =__stack_abt_end
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c, r1
    ldr sp, =__stack_und_end
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x1F
    msr cpsr_c, r1
    ldr sp, =__stack_sys_end
    
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
    ldr sp, =__stack_svc_end
    
    bx lr

.size arm_init_stacks, . - arm_init_stacks