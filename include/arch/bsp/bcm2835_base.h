/**
 * @file bcm2835_base.h
 * @brief Common BCM2835 peripheral base addresses.
 * @defgroup bcm2835_base BCM2835 Address Map
 * @brief Bus and physical address definitions for BCM2835 peripherals.
 *
 * BCM2835 peripherals use bus addresses (0x7Exxxxxx) that must be
 * converted to physical addresses by subtracting the bus offset.
 * On Raspberry Pi 2B, the offset is 0x3F000000.
 * @{
 */

#ifndef BCM2835_BASE_H
#define BCM2835_BASE_H

/** @brief Bus address base from BCM2835 ARM Peripherals manual. */
#define BCM2835_PERIPH_BASE_BUS         0x7E000000u

/** @brief Bus-to-physical offset on Raspberry Pi 2B. */
#define BCM2835_PERIPH_BUS_OFFSET       0x3F000000u

/** @brief System Timer bus address. */
#define BCM2835_SYSTIMER_BASE_BUS       (BCM2835_PERIPH_BASE_BUS + 0x00003000u)

/** @brief PL011 UART bus address. */
#define BCM2835_PL011_BASE_BUS          (BCM2835_PERIPH_BASE_BUS + 0x00201000u)

/** @brief System Timer physical address. */
#define BCM2835_SYSTIMER_BASE_PHYS      (BCM2835_SYSTIMER_BASE_BUS - BCM2835_PERIPH_BUS_OFFSET)

/** @brief PL011 UART physical address. */
#define BCM2835_PL011_BASE_PHYS         (BCM2835_PL011_BASE_BUS - BCM2835_PERIPH_BUS_OFFSET)

/** @brief IRQ controller bus address. */
#define BCM2835_IRQCTRL_BASE_BUS        (BCM2835_PERIPH_BASE_BUS + 0x0000B200u)

/** @brief IRQ controller physical address. */
#define BCM2835_IRQCTRL_BASE_PHYS       (BCM2835_IRQCTRL_BASE_BUS - BCM2835_PERIPH_BUS_OFFSET)

#endif /* BCM2835_BASE_H */
/** @} */ /* end of bcm2835_base */