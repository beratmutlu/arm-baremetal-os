
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

#define SYSTIMER_BASE_BUS		0x7E003000u /**< Bus address from BCM2835 manual. */
#define PERIPH_BUS_OFFSET 	    0x3F000000u /**< Bus→phys offset on RPi2b. */


/**
 * @def SYSTIMER_BASE_PHYS
 * @brief Physical base address of the Systimer peripheral.
 *
 * The bus address (0x7E003000u) must be converted to a physical address by
 * subtracting the peripheral offset (0x3F000000) on BCM2835.
 */
#define SYSTIMER_BASE_PHYS       (SYSTIMER_BASE_BUS - PERIPH_BUS_OFFSET)

/**
 * @struct systimer_regs
 * @brief Memory-mapped Systimer register block.
 *
 * Use @ref SYSTIMER to access the device instance.
 */
struct systimer_regs {
    volatile uint32_t CS;
    volatile uint32_t CLO;
    volatile uint32_t CHI;
    volatile uint32_t C0;
    volatile uint32_t C1;
    volatile uint32_t C2;
    volatile uint32_t C3;
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
#define SYSTIMER   ((volatile struct systimer_regs *)(uintptr_t)SYSTIMER_BASE_PHYS)


#endif /* BCM2835_SYSTIMER_REGS_H */
/** @} */ /* end of systimer_bsp */