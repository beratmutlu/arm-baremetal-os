#ifndef MMU_TRIGGERS_H
#define MMU_TRIGGERS_H

extern char ld_section_kernel_text;
extern char ld_section_kernel_data_bss;


/** Bus-to-physical offset on Raspberry Pi 2B. */
#define BCM2835_PERIPH_BUS_OFFSET       0x3F000000u

/**
 * Read from kernel data section
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_data(void) {
    volatile char var = *((volatile char *)&ld_section_kernel_data_bss);
    (void)var;
}

/**
 * Read from kernel text section
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_text(void) {
    volatile char var = *((volatile char *)&ld_section_kernel_text);
    (void)var;
}

/**
 * Read from kernel stack area
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_stack(void) {
    volatile char var = *((volatile char *)0x004FF000u);
    (void)var;
}

/**
 * Read from MMIO (peripheral) region
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_mmio(void) {
    volatile uint32_t var = *((volatile uint32_t *)BCM2835_PERIPH_BUS_OFFSET);
    (void)var;
}

/**
 * Read from an invalid/unmapped address
 * Triggers data abort due to translation fault
 */
static inline void read_invalid_addr(void) {
    volatile char var = *((volatile char *)0xFFFFFFFFu);
    (void)var;
}

#endif