
/**
 * @file pl011_regs.h
 * @brief PL011 UART register map and tiny MMIO helpers.
 *
 * Defines the memory-mapped register block of the ARM PL011 and a few
 * minimal inline helpers for flag checks. Higher-level policy lives in uart.c.
 * 
 */

 /** @defgroup uart_bsp PL011 BSP (low-level)
 *  @brief Low-level PL011 register definitions and helpers.
 *  @{
 */

#ifndef PL011_REGS_H
#define PL011_REGS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#define PL011_BASE_BUS		    0x7E201000u /**< Bus address from BCM2835 manual. */
#define PERIPH_BUS_OFFSET 	    0x3F000000u /**< Bus→phys offset on RPi2b. */

/**
 * @def PL011_BASE_PHYS
 * @brief Physical base address of the PL011 UART peripheral.
 *
 * The bus address (0x7E201000) must be converted to a physical address by
 * subtracting the peripheral offset (0x3F000000) on BCM2835.
 */
#define PL011_BASE_PHYS	        (PL011_BASE_BUS - PERIPH_BUS_OFFSET)

/**
 * @enum pl011_flag_bits
 * @brief Bit positions of the UART Flag Register (FR).
 */
enum {
    FR_RXFE = 1u << 4,  /**< Receive FIFO empty. */
    FR_TXFF = 1u << 5   /**< Transmit FIFO full. */   
};

/**
 * @enum pl011_interrupt_bits
 * @brief Bit positions of the UART Interrupt Registers.
 */
enum {
    INT_RX = 1u << 4,   /* RX interrupt. */
    INT_TX = 1u << 5,   /* TX interrupt. */
    INT_RT = 1u << 6    /* RX timeout interrupt. */
};

/**
 * @struct pl011_regs
 * @brief Memory-mapped PL011 register block.
 *
 * Use @ref PL011 to access the device instance.
 */
struct pl011_regs {
    volatile uint32_t DR;            /**<  Data Register                          (0x00). */
    volatile uint32_t RSR_ECR;       /**<  Receive Status / Error Clear           (0x04). */
    volatile uint32_t _rsvd0[4];     /**<  Reserved                               (0x08–0x17). */
    volatile uint32_t FR;            /**<  Flag Register                          (0x18). */
    volatile uint32_t unused1[7];    /**<  Unused                                 (0x1C-0x34). */
    volatile uint32_t IMSC;          /**<  Interrupt Mask Set/Clear               (0x38). */
    volatile uint32_t unused2;       /**<  Unused                                 (0x3C). */
    volatile uint32_t MIS;           /**<  Masked Interrupt Status                (0x40). */
    volatile uint32_t ICR;           /**<  Interrupt Clear                        (0x44). */   
};

_Static_assert(offsetof(struct pl011_regs, DR)      == 0x00, "PL011: DR OFFSET");
_Static_assert(offsetof(struct pl011_regs, RSR_ECR) == 0x04, "PL011: RSR/ECR OFFSET");
_Static_assert(offsetof(struct pl011_regs, FR)      == 0x18, "PL011: FR OFFSET");
_Static_assert(offsetof(struct pl011_regs, unused1) == 0x1C, "PL011: unused1 offset");
_Static_assert(offsetof(struct pl011_regs, IMSC)    == 0x38, "PL011: IMSC offset");
_Static_assert(offsetof(struct pl011_regs, unused2) == 0x3C, "PL011: unused2 offset");
_Static_assert(offsetof(struct pl011_regs, MIS)     == 0x40, "PL011: MIS offset");
_Static_assert(offsetof(struct pl011_regs, ICR)     == 0x44, "PL011: ICR offset");
_Static_assert(sizeof(struct pl011_regs)            == 0x48, "PL011: struct size should reach 0x48");

/**
 * @brief Base pointer to the UART hardware registers.
 */
#define PL011   ((volatile struct pl011_regs *)(uintptr_t)PL011_BASE_PHYS)

/**
 * @brief Check whether the transmit FIFO is full.
 * @return `true` if FIFO is full, `false` otherwise.
 */
static inline bool pl011_tx_full(void) {
    return (PL011->FR & FR_TXFF) != 0u;
}

/**
 * @brief Check whether the receive FIFO is empty.
 * @return `true` if FIFO is empty, `false` otherwise.
 */
static inline bool pl011_rx_empty(void){
    return (PL011->FR & FR_RXFE) != 0u;
}

#endif /* PL011_REGS_H */
/** @} */ /* end of uart_bsp */