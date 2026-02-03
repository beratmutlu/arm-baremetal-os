#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mmu_asid_t;

#define MMU_AS_MAX_COUNT  8u
#define MMU_ASID_INVALID  ((mmu_asid_t)0xFFu)

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


mmu_asid_t mmu_as_create(void);


void mmu_as_retain(mmu_asid_t asid);


void mmu_as_release(mmu_asid_t asid);


void mmu_switch_as(mmu_asid_t asid);

#endif /* MMU_H */
