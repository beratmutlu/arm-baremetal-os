#include "kernel/exc_frame_layout.h"

.syntax unified
.arch armv7-a
.arm

.text

.extern und_handler_c
.extern svc_handler_c
.extern pabt_handler_c
.extern dabt_handler_c
.extern irq_handler_c


/*
 * EXC_ENTRY
 *
 * On entry:
 *   - CPU in exception mode (UND/SVC/ABT/IRQ)
 *   - LR = return address + mode-specific adjustment
 *   - SPSR_<mode> = CPSR snapshot at exception time
 *
 * On exit:
 *   - SP points to newly reserved exc_frame
 *   - Memory layout matches struct exc_frame (see exc_frame_layout.h)
 */
.macro EXC_ENTRY
    SUB     SP, SP, #EXC_FRAME_TOTAL_BYTES  /**< reserve frame */
    STMIA   SP, {R0-R12}                    /**< save r0-r12 */
    MRS     R1, SPSR
    STR     LR, [SP, #OFF_LR]               /**< save LR at it's ABI-defined offset */
    STR     R1, [SP, #OFF_SPSR]             /**< save SPSR at it's ABI-defined offset*/
.endm

/*
 * EXC_EXIT adjust
 *
 * Restores registers from the frame and returns from the exception.
 * 'adjust' is the amount to subtract from LR before restoring PC, if needed.
 */
.macro EXC_EXIT adjust=4
    LDR     R1, [SP, #OFF_SPSR]             /**< restore SPSR first*/
    MSR     SPSR_cxsf, R1

    LDMIA   SP, {R0-R12, LR}                /**< restore r0-r12 and LR */

    ADD     SP, SP, #EXC_FRAME_TOTAL_BYTES  /**< pop frame */
    .if \adjust == 0                        /**< straight return */
        MOVS    PC, LR
    .else                                   /**< LR already points past the trapping instruction */
        SUBS    PC, LR, #\adjust
    .endif
.endm

.global und_tramp, svc_tramp, pabt_tramp, dabt_tramp, irq_tramp

und_tramp:
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      und_handler_c
    EXC_EXIT 0
svc_tramp:
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      svc_handler_c
    EXC_EXIT 0
pabt_tramp:
    SUB     LR, LR, #4
    EXC_ENTRY
    MOV     R0, SP
    BL      pabt_handler_c
    EXC_EXIT 0
dabt_tramp:
    SUB     LR, LR, #8
    EXC_ENTRY
    MOV     R0, SP
    BL      dabt_handler_c
    EXC_EXIT 0
irq_tramp:
    EXC_ENTRY
    MOV     R0, SP
    BL      irq_handler_c
    EXC_EXIT 4

.global scheduler_start_asm
scheduler_start_asm:
    MOV     SP, R0                          
    EXC_EXIT 0                              