/**
 * @file exc_frame_layout.h
 * @defgroup exc_frame_abi Exception frame ABI
 * @brief Layout of exception frames shared between C and traps.S.
 *
 * This module defines the exact C struct layout that traps.S uses when
 * saving registers on exception entry. Any change here must be mirrored
 * in traps.S.
 * @{
 */

#ifndef EXC_FRAME_LAYOUT_H
#define EXC_FRAME_LAYOUT_H

/** Size of each exception mode stack in bytes. */
#define EXC_STACK_SIZE         4096u

#define EXC_WORD_SIZE          4

/* General-purpose registers + LR + SPSR */
#define EXC_GPR_COUNT         13      /* r0-r12 */
#define EXC_LR_INDEX          13
#define EXC_SPSR_INDEX        14

#define OFF_R0               (0  * EXC_WORD_SIZE)
#define OFF_R12              (12 * EXC_WORD_SIZE)

#define OFF_LR               (EXC_LR_INDEX   * EXC_WORD_SIZE)   /* 13 * 4 = 52 */
#define OFF_SPSR             (EXC_SPSR_INDEX * EXC_WORD_SIZE)   /* 14 * 4 = 56 */

/* Total frame size in words/bytes */
#define EXC_FRAME_TOTAL_WORDS  (EXC_SPSR_INDEX + 1)             /* 15 words  */
#define EXC_FRAME_TOTAL_BYTES  (EXC_FRAME_TOTAL_WORDS * EXC_WORD_SIZE)


#ifndef __ASSEMBLER__


#include <stdint.h>
#include <stddef.h>

struct exc_frame {
    uint32_t r[EXC_GPR_COUNT];  /* r0–r12 */
    uint32_t lr;                /* exception mode LR */
    uint32_t spsr;              /* exception mode SPSR */
};


/* Compile-time layout validation (C <-> ASM ABI contract) */
_Static_assert(offsetof(struct exc_frame, r[0]) == OFF_R0,
               "exc_frame: r0 offset mismatch");
_Static_assert(offsetof(struct exc_frame, r[12]) == OFF_R12,
               "exc_frame: r12 offset mismatch");
_Static_assert(offsetof(struct exc_frame, lr) == OFF_LR,
               "exc_frame: lr offset mismatch");
_Static_assert(offsetof(struct exc_frame, spsr) == OFF_SPSR,
               "exc_frame: spsr offset mismatch");
_Static_assert(sizeof(struct exc_frame) == EXC_FRAME_TOTAL_BYTES,
               "exc_frame: total size mismatch");

               
/**
 * @brief Get link register from exception frame.
 * @param f Exception frame pointer
 * @return Link register value
 */              
uint32_t exc_frame_get_lr(const struct exc_frame *f);

/**
 * @brief Get stack pointer before exception.
 * @param f Exception frame pointer
 * @return Stack pointer value (points after the saved frame)
 */
uint32_t exc_frame_get_sp(const struct exc_frame *f);

/**
 * @brief Get saved program status register.
 * @param f Exception frame pointer
 * @return SPSR value
 */
uint32_t exc_frame_get_spsr(const struct exc_frame *f);

/**
 * @brief Get general-purpose register from exception frame.
 * @param f Exception frame pointer
 * @param i Register index (0-12 for r0-r12)
 * @return Register value
 */
uint32_t exc_frame_get_r(const struct exc_frame *f, unsigned i);

#endif /* !__ASSEMBLER__ */


#endif /* EXC_FRAME_LAYOUT_H */
/** @} */ /* end of exc_frame_abi */

