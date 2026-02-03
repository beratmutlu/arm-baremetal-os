#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
#include <stdint.h>


typedef uint32_t l1_entry;


enum mmu_permission {
    PERM_NO_ACCESS   = 0b000,
    PERM_RW_NA       = 0b001,
    PERM_RW_R        = 0b010,
    PERM_FULL_ACCESS = 0b011,
    PERM_R_NA        = 0b101,
    PERM_R_R         = 0b111,
};

#define MMU_SECTION_SHIFT        20u
#define MMU_SECTION_SIZE         (1u << MMU_SECTION_SHIFT)   /* 1 MiB */
#define MMU_SMALL_PAGE_SHIFT     12u
#define MMU_SMALL_PAGE_SIZE      (1u << MMU_SMALL_PAGE_SHIFT) /* 4 KiB */

#define MMU_L1_ENTRIES           4096u
#define MMU_L1_TABLE_ALIGNMENT   (16u * 1024u)

/* Coarse L2: 256 entries = 1 MiB window via 4 KiB pages */
#define MMU_L2_ENTRIES           256u
#define MMU_L2_TABLE_ALIGNMENT   1024u


[[nodiscard]] l1_entry mmu_l1_section(void *phy_addr, enum mmu_permission perm, bool xn, bool pxn);


[[nodiscard]] l1_entry mmu_l1_fault [[gnu::const]] (void);


void mmu_set_l1_entry(void *virt_addr, l1_entry entry);

typedef uint32_t l2_entry;


[[nodiscard]] l1_entry mmu_l1_page_table(void *l2_table);


[[nodiscard]] l2_entry mmu_l2_small_page(void *phy_addr, enum mmu_permission perm, bool xn);


[[nodiscard]] l2_entry mmu_l2_fault(void);


void mmu_init(void);


void mmu_setup_protection(void);


void mmu_enable(void);

/* Address space support */
#define AS_MAX 9u   /* 1 boot AS + 8 user AS */
#define AS_INVALID (~0u)

uint32_t mmu_as_create(void);
void mmu_as_addref(uint32_t asid);
void mmu_as_release(uint32_t asid);
uint32_t mmu_as_get_refcount(uint32_t asid);
void mmu_as_switch(uint32_t asid);
uint32_t mmu_as_get_boot(void);

#endif /* MMU_H */
