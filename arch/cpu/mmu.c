#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <arch/cpu/mmu.h>
#include <arch/bsp/uart.h>
#include <kernel/panic.h>
#include <kernel/diagnose_mmu.h>
#include <kernel/threads.h>

extern unsigned char thread_stacks_phys[THREADS_MAX_COUNT][THREADS_STACK_PAGE_SIZE];


extern char ld_section_kernel_text;
extern char ld_section_kernel_text_end;
extern char ld_section_kernel_rodata;
extern char ld_section_kernel_rodata_end;
extern char ld_section_kernel_data_bss;
extern char ld_section_kernel_data_bss_end;
extern char ld_section_user_text;
extern char ld_section_user_text_end;
extern char ld_section_user_rodata;
extern char ld_section_user_rodata_end;
extern char ld_section_user_data_bss;
extern char ld_section_user_data_bss_end;


#define MMU_USABLE_RAM_SIZE_BYTES   (128u * 1024u * 1024u)


#define MMU_PERIPH_BASE             0x3F000000u
#define MMU_PERIPH_SIZE_BYTES       (16u * 1024u * 1024u)


#define MMU_ALIGN_MASK(sz)          ((sz) - 1u)

/* L1 section descriptor */
#define L1_DESC_TYPE_SECTION        0x2u
#define L1_SECTION_BASE_MASK        0xFFF00000u
#define L1_SECTION_DOMAIN_SHIFT     5u
#define L1_SECTION_XN_BIT           (1u << 4)
#define L1_SECTION_AP01_SHIFT       10u
#define L1_SECTION_AP2_BIT          (1u << 15)
#define L1_SECTION_PXN_BIT          (1u << 0)

/* L1 page table descriptor (pointing to L2) */
#define L1_DESC_TYPE_PAGE_TABLE     0x1u
#define L1_PAGE_TABLE_BASE_MASK     0xFFFFFC00u
#define L1_PAGE_TABLE_DOMAIN_SHIFT  5u

/* L2 small page descriptor */
#define L2_DESC_TYPE_SMALL_PAGE     0x2u
#define L2_SMALL_PAGE_BASE_MASK     0xFFFFF000u
#define L2_SMALL_PAGE_XN_BIT        (1u << 0)
#define L2_SMALL_PAGE_AP01_SHIFT    4u
#define L2_SMALL_PAGE_AP2_BIT       (1u << 9)

/* Domain configuration (DACR) */
#define MMU_DOMAIN_KERNEL           0u
#define MMU_DOMAIN_USER_EXEC        1u  

#define DACR_DOMAIN_MODE_NO_ACCESS  0x0u
#define DACR_DOMAIN_MODE_CLIENT     0x1u
#define DACR_DOMAIN_MODE_MANAGER    0x3u
#define DACR_DOMAIN_MODE(domain, mode)   ((uint32_t)(mode) << (2u * (uint32_t)(domain)))

/* SCTLR */
#define SCTLR_MMU_ENABLE_BIT        (1u << 0)



static l1_entry l1_page_table[MMU_L1_ENTRIES] __attribute__((aligned(MMU_L1_TABLE_ALIGNMENT)));

typedef struct l2_table_aligned {
    l2_entry e[MMU_L2_ENTRIES];
} __attribute__((aligned(MMU_L2_TABLE_ALIGNMENT))) l2_table_t;

static l2_table_t l2_stack_tables[THREADS_MAX_COUNT];

static inline bool is_aligned_uint(uint32_t addr, uint32_t alignment_bytes) {
    return (addr & MMU_ALIGN_MASK(alignment_bytes)) == 0u;
}


l2_entry mmu_l2_fault(void) {
    return 0u;
}

static inline uint32_t small_page_ap_bits(enum mmu_permission perm) {
    const uint32_t ap = (uint32_t)perm;
    
    uint32_t bits = 0u;
    bits |= ((ap & 0x3u) << L2_SMALL_PAGE_AP01_SHIFT);
    if ((ap & 0x4u) != 0u) {
        bits |= L2_SMALL_PAGE_AP2_BIT;
    }
    return bits;
}

l2_entry mmu_l2_small_page(void *phy_addr, enum mmu_permission perm, bool xn) {
    const uint32_t pa = (uint32_t)(uintptr_t)phy_addr;
    
    if (!is_aligned_uint(pa, MMU_SMALL_PAGE_SIZE)) {
        panic("MMU: phys addr not 4KiB aligned");
    }
    
    uint32_t desc = 0u;
    desc |= (pa & L2_SMALL_PAGE_BASE_MASK);
    desc |= L2_DESC_TYPE_SMALL_PAGE;
    desc |= small_page_ap_bits(perm);
    
    if (xn) {
        desc |= L2_SMALL_PAGE_XN_BIT;
    }
    
    return desc;
}

l1_entry mmu_l1_page_table(void *l2_table, uint32_t domain) {
    const uint32_t addr = (uint32_t)(uintptr_t)l2_table;
    
    if (!is_aligned_uint(addr, MMU_L2_TABLE_ALIGNMENT)) {
        panic("MMU: L2 table not 1KiB aligned");
    }
    
    uint32_t desc = 0u;
    desc |= (addr & L1_PAGE_TABLE_BASE_MASK);
    desc |= L1_DESC_TYPE_PAGE_TABLE;
    desc |= ((domain & 0xFu) << L1_PAGE_TABLE_DOMAIN_SHIFT);
    
    return desc;
}
l1_entry mmu_l1_fault(void) {
    return 0u;
}

void mmu_set_l1_entry(void *virt_addr, l1_entry entry) {
    const uint32_t v = (uint32_t)(uintptr_t)virt_addr;

    if (!is_aligned_uint(v, MMU_SECTION_SIZE)) {
        panic("MMU: virt addr not 1MiB aligned");
    }

    l1_page_table[v >> MMU_SECTION_SHIFT] = entry;
}

static inline uint32_t section_ap_bits(enum mmu_permission perm) {
    const uint32_t ap = (uint32_t)perm;

    uint32_t bits = 0u;
    bits |= ((ap & 0x3u) << L1_SECTION_AP01_SHIFT);
    if ((ap & 0x4u) != 0u) {
        bits |= L1_SECTION_AP2_BIT;
    }
    return bits;
}

l1_entry mmu_l1_section(void *phy_addr, enum mmu_permission perm, 
                        bool xn, bool pxn, uint32_t domain) {
    const uint32_t pa = (uint32_t)(uintptr_t)phy_addr;
    if (!is_aligned_uint(pa, MMU_SECTION_SIZE)) {
        panic("MMU: phys addr not 1MiB aligned");
    }

    uint32_t desc = 0u;
    desc |= (pa & L1_SECTION_BASE_MASK);
    desc |= L1_DESC_TYPE_SECTION;
    desc |= ((domain & 0xFu) << L1_SECTION_DOMAIN_SHIFT);
    desc |= section_ap_bits(perm);
    if (xn) desc |= L1_SECTION_XN_BIT;
    if (pxn) desc |= L1_SECTION_PXN_BIT;  
    return desc;
}



static inline void cp15_write_dacr(uint32_t v) {
    asm volatile("mcr p15, 0, %0, c3, c0, 0" :: "r"(v) : "memory");
}

static inline void cp15_write_ttbcr(uint32_t v) {
    asm volatile("mcr p15, 0, %0, c2, c0, 2" :: "r"(v) : "memory");
}

static inline void cp15_write_ttbr0(uint32_t v) {
    asm volatile("mcr p15, 0, %0, c2, c0, 0" :: "r"(v) : "memory");
}

static inline uint32_t cp15_read_sctlr(void) {
    uint32_t v;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(v));
    return v;
}

static inline void cp15_write_sctlr(uint32_t v) {
    asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(v) : "memory");
}

static inline void barrier_dsb(void) {
    asm volatile("dsb sy" ::: "memory");
}

static inline void barrier_isb(void) {
    asm volatile("isb" ::: "memory");
}

static inline void tlb_invalidate_all(void) {
    asm volatile("mcr p15, 0, %0, c8, c7, 0" :: "r"(0u) : "memory");
}

static void l1_fill_fault(void) {
    for (uint32_t i = 0; i < MMU_L1_ENTRIES; i++) {
        l1_page_table[i] = mmu_l1_fault();
    }
}

static void map_identity_range(uint32_t start_addr, uint32_t size_bytes,
                               enum mmu_permission perm, bool xn, bool pxn,
                               uint32_t domain) 
{
    const uint32_t first_mb = start_addr >> MMU_SECTION_SHIFT;
    const uint32_t last_addr = start_addr + size_bytes;
    const uint32_t last_mb_exclusive = (last_addr + MMU_ALIGN_MASK(MMU_SECTION_SIZE)) >> MMU_SECTION_SHIFT;

    for (uint32_t mb = first_mb; mb < last_mb_exclusive; mb++) {
        const uint32_t base = mb << MMU_SECTION_SHIFT;
        mmu_set_l1_entry((void *)(uintptr_t)base,
                         mmu_l1_section((void *)(uintptr_t)base, perm, xn, pxn, domain));
    }
}

void mmu_init(void) {
    l1_fill_fault();

    /* Initial 1:1 mapping with FULL access for diagnostic check */
    for (uint32_t mb = 0; mb < MMU_L1_ENTRIES; mb++) {
        const uint32_t base = mb << MMU_SECTION_SHIFT;
        mmu_set_l1_entry((void *)(uintptr_t)base,
                         mmu_l1_section((void *)(uintptr_t)base, 
                                       PERM_FULL_ACCESS, false, false, MMU_DOMAIN_KERNEL));
    }

    cp15_write_ttbcr(0u);
    cp15_write_ttbr0((uint32_t)(uintptr_t)l1_page_table);
    cp15_write_dacr(DACR_DOMAIN_MODE(MMU_DOMAIN_KERNEL, DACR_DOMAIN_MODE_CLIENT));

    barrier_dsb();
    tlb_invalidate_all();
    barrier_dsb();
    barrier_isb();

    check_mmu_1_to_1(l1_page_table);
}
void mmu_setup_protection(void) {
    l1_fill_fault();

    const uint32_t kernel_text_start = (uint32_t)(uintptr_t)&ld_section_kernel_text;
    const uint32_t kernel_text_end = (uint32_t)(uintptr_t)&ld_section_kernel_text_end;
    const uint32_t kernel_rodata_start = (uint32_t)(uintptr_t)&ld_section_kernel_rodata;
    const uint32_t kernel_rodata_end = (uint32_t)(uintptr_t)&ld_section_kernel_rodata_end;
    const uint32_t kernel_data_bss_start = (uint32_t)(uintptr_t)&ld_section_kernel_data_bss;
    const uint32_t kernel_data_bss_end = (uint32_t)(uintptr_t)&ld_section_kernel_data_bss_end;
    
    const uint32_t user_text_start = (uint32_t)(uintptr_t)&ld_section_user_text;
    const uint32_t user_text_end = (uint32_t)(uintptr_t)&ld_section_user_text_end;
    const uint32_t user_rodata_start = (uint32_t)(uintptr_t)&ld_section_user_rodata;
    const uint32_t user_rodata_end = (uint32_t)(uintptr_t)&ld_section_user_rodata_end;
    const uint32_t user_data_bss_start = (uint32_t)(uintptr_t)&ld_section_user_data_bss;
    const uint32_t user_data_bss_end = (uint32_t)(uintptr_t)&ld_section_user_data_bss_end;

/* Map first 1 MiB (contains .init and boot stack) */
/* Must be PERM_RW_NA so the kernel can still use its stack! */
    map_identity_range(0x0, 0x100000, PERM_RW_NA, false, false, MMU_DOMAIN_KERNEL);

    /* Kernel text: Manager domain → kernel can execute, user gets fault on access */
    map_identity_range(kernel_text_start, kernel_text_end - kernel_text_start,
                    PERM_R_NA, false, false, MMU_DOMAIN_KERNEL);

    /* Kernel rodata: Manager domain */
    map_identity_range(kernel_rodata_start, kernel_rodata_end - kernel_rodata_start,
                    PERM_R_NA, true, false, MMU_DOMAIN_KERNEL);

    /* Kernel data/bss: Manager domain */
    map_identity_range(kernel_data_bss_start, kernel_data_bss_end - kernel_data_bss_start,
                    PERM_RW_NA, true, false, MMU_DOMAIN_KERNEL);

    /* User text: Client domain → permission checks apply */
    map_identity_range(user_text_start, user_text_end - user_text_start,
                    PERM_R_R, false, true, MMU_DOMAIN_USER_EXEC);

    /* User rodata: Client domain */
    map_identity_range(user_rodata_start, user_rodata_end - user_rodata_start,
                    PERM_R_R, true, false, MMU_DOMAIN_USER_EXEC);

    /* User data/bss: Client domain */
    map_identity_range(user_data_bss_start, user_data_bss_end - user_data_bss_start,
                    PERM_FULL_ACCESS, true, false, MMU_DOMAIN_USER_EXEC);

    /* Peripherals: Manager domain */
    map_identity_range(MMU_PERIPH_BASE, MMU_PERIPH_SIZE_BYTES,
                      PERM_RW_NA, true, false, MMU_DOMAIN_KERNEL);
    
    /* Set up L2 page tables for thread stacks */
    for (uint32_t tid = 0; tid < THREADS_MAX_COUNT; tid++) {
        const uint32_t win_base = THREADS_STACK_WIN_BASE(tid);

        /* Point L1 entry to this thread's L2 table (use USER_EXEC domain) */
        mmu_set_l1_entry((void *)(uintptr_t)win_base,
                         mmu_l1_page_table(l2_stack_tables[tid].e, MMU_DOMAIN_USER_EXEC));

        /* Initialize ALL L2 entries to fault (ensures upper guard) */
        for (uint32_t i = 0; i < MMU_L2_ENTRIES; i++) {
            l2_stack_tables[tid].e[i] = mmu_l2_fault();
        }

        /* Page 0: Lower guard (already fault from loop above) */
        /* Page 1: Actual stack - 4 KiB at virtual offset 0x1000 */
        l2_stack_tables[tid].e[1] = mmu_l2_small_page(&thread_stacks_phys[tid][0],
                                                     PERM_FULL_ACCESS,
                                                     true);
        /* Page 2+: Upper guards (already fault from loop above) */
    }

    barrier_dsb();
    tlb_invalidate_all();
    barrier_dsb();
    barrier_isb();
    
    /* Re-write TTBR0 to force TLB context switch */
    cp15_write_ttbr0((uint32_t)(uintptr_t)l1_page_table);
    barrier_isb();
}

void mmu_enable(void) {
    cp15_write_ttbcr(0u);
    cp15_write_ttbr0((uint32_t)(uintptr_t)l1_page_table);
    
    /* Both domains as Client to enforce permission checks */
    cp15_write_dacr(
        DACR_DOMAIN_MODE(MMU_DOMAIN_KERNEL, DACR_DOMAIN_MODE_CLIENT) |
        DACR_DOMAIN_MODE(MMU_DOMAIN_USER_EXEC, DACR_DOMAIN_MODE_CLIENT)
    );

    barrier_dsb();
    tlb_invalidate_all();
    barrier_dsb();
    barrier_isb();

    uint32_t sctlr = cp15_read_sctlr();
    sctlr |= SCTLR_MMU_ENABLE_BIT;
    cp15_write_sctlr(sctlr);
    barrier_isb();
}