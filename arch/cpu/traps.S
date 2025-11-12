.syntax unified
.arch armv7-a
.arm

.text

.extern und_handler_c
.extern svc_handler_c
.extern pabt_handler_c
.extern dabt_handler_c
.extern irq_handler_c

#define FRAME_SIZE (15 * 4)

.macro EXC_ENTRY
    SUB     SP, SP, #FRAME_SIZE
    STMIA   SP, {R0-R12}
    MRS     R1, SPSR
    STR     LR, [SP, #52]
    STR     R1, [SP, #56]
    CPSID   I
.endm

.macro EXC_EXIT adjust=4
    LDR     R1, [SP, #56]
    MSR     SPSR_cxsf, R1
    LDMIA   SP, {R0-R12, LR}
    ADD     SP, SP, #FRAME_SIZE
    .if \adjust == 0
        MOVS    PC, LR
    .else
        SUBS    PC, LR, #\adjust
    .endif
.endm

.global und_tramp, svc_tramp, pabt_tramp, dabt_tramp, irq_tramp

und_tramp:
    CPSID   I
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      und_handler_c

svc_tramp:
    CPSID   I
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      svc_handler_c

pabt_tramp:
    CPSID   I
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      pabt_handler_c

dabt_tramp:
    CPSID   I
    SUB     LR, LR, #8
    EXC_ENTRY
    MOV     R0, SP
    BL      dabt_handler_c

irq_tramp:
    EXC_ENTRY
    MOV     R0, SP
    BL      irq_handler_c
    EXC_EXIT 4