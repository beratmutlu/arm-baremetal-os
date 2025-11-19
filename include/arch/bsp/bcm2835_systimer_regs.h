
/**
 * @file bcm2835_systimer_regs.h
 * @brief BCM2835 Systimer register map and tiny MMIO helpers.
 *
 * Defines the memory-mapped register block of the ARM BCM2835 Systimer and a few
 * minimal inline helpers for flag checks. Higher-level policy lives in systimer.c.
 * 
 */

 /** @defgroup systimer_bsp BCM2835 Systimer BSP (low-level)
 *  @brief Low-level BCM2835 Systimer register definitions and helpers.
 *  @{
 */

#ifndef BCM2835_SYSTIMER_REGS_H
#define BCM2835_SYSTIMER_REGS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <arch/bsp/bcm2835_base.h>

#define SYSTIMER_M1 (1u << 1)

/**
 * @struct systimer_regs
 * @brief Memory-mapped Systimer register block.
 *
 * Use @ref SYSTIMER to access the device instance.
 */
struct systimer_regs {
    volatile uint32_t CS;   /**< Control/Status (0x00). */
    volatile uint32_t CLO;  /**< Counter Low 32 bits (0x04). */
    volatile uint32_t CHI;  /**< Counter High 32 bits (0x08). */
    volatile uint32_t C0;   /**< Compare 0 (0x0C). */
    volatile uint32_t C1;   /**< Compare 1 (0x10). */
    volatile uint32_t C2;   /**< Compare 2 (0x14). */
    volatile uint32_t C3;   /**< Compare 3 (0x18). */
};


_Static_assert(offsetof(struct systimer_regs, CS)   == 0x00, "SYSTIMER: CS OFFSET");
_Static_assert(offsetof(struct systimer_regs, CLO)  == 0x04, "SYSTIMER: CLO OFFSET");
_Static_assert(offsetof(struct systimer_regs, CHI)  == 0x08, "SYSTIMER: CHI OFFSET");
_Static_assert(offsetof(struct systimer_regs, C0)   == 0x0C, "SYSTIMER: C0 offset");
_Static_assert(offsetof(struct systimer_regs, C1)   == 0x10, "SYSTIMER: C1 offset");
_Static_assert(offsetof(struct systimer_regs, C2)   == 0x14, "SYSTIMER: C2 offset");
_Static_assert(offsetof(struct systimer_regs, C3)   == 0x18, "SYSTIMER: C3 offset");
_Static_assert(sizeof(struct systimer_regs)         == 0x1C, "SYSTIMER: struct size should reach 0x48");

/**
 * @brief Base pointer to the Systimer hardware registers.
 */
#define SYSTIMER   ((volatile struct systimer_regs *)(uintptr_t)BCM2835_SYSTIMER_BASE_PHYS)


#endif /* BCM2835_SYSTIMER_REGS_H */
/** @} */ /* end of systimer_bsp */