.syntax unified
.arch armv7-a
.arm

.section .vectors, "ax"
.align 5
.global vectors_table

vectors_table:
    b .
    b und_tramp
    b svc_tramp
    b pabt_tramp
    b dabt_tramp
    b .
    b irq_tramp
    b .

.text
.global arm_set_vbar
.type arm_set_vbar, %function

arm_set_vbar:
    mcr p15, 0, r0, c12, c0, 0
    bx lr
