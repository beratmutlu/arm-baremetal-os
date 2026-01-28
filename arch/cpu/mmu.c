#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <arch/cpu/mmu.h>
#include <arch/bsp/uart.h>
#include <kernel/panic.h>
#include <kernel/diagnose_mmu.h>
#include <kernel/threads.h>

extern unsigned char thread_stacks_phys[THREADS_MAX_COUNT][THREADS_STACK_PAGE_SIZE];

extern unsigned char svc_stack_guard_lower[THREADS_STACK_PAGE_SIZE];
extern unsigned char svc_stack[THREADS_STACK_PAGE_SIZE];
extern unsigned char svc_stack_guard_upper[THREADS_STACK_PAGE_SIZE];

extern unsigned char irq_stack_guard_lower[THREADS_STACK_PAGE_SIZE];
extern unsigned char irq_stack[THREADS_STACK_PAGE_SIZE];
extern unsigned char irq_stack_guard_upper[THREADS_STACK_PAGE_SIZE];

extern unsigned char abt_stack_guard_lower[THREADS_STACK_PAGE_SIZE];
extern unsigned char abt_stack[THREADS_STACK_PAGE_SIZE];
extern unsigned char abt_stack_guard_upper[THREADS_STACK_PAGE_SIZE];

extern unsigned char und_stack_guard_lower[THREADS_STACK_PAGE_SIZE];
extern unsigned char und_stack[THREADS_STACK_PAGE_SIZE];
extern unsigned char und_stack_guard_upper[THREADS_STACK_PAGE_SIZE];

extern unsigned char sys_stack_guard_lower[THREADS_STACK_PAGE_SIZE];
extern unsigned char sys_stack[THREADS_STACK_PAGE_SIZE];
extern unsigned char sys_stack_guard_upper[THREADS_STACK_PAGE_SIZE];

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

extern char ld_section_init_start;
extern char ld_section_init_end;

#define MMU_USABLE_RAM_SIZE_BYTES   (128u * 1024u * 1024u)
#define MMU_PERIPH_BASE             0x3F000000u
#define MMU_PERIPH_SIZE_BYTES       (16u * 1024u * 1024u)
#define MMU_ALIGN_MASK(sz)          ((sz) - 1u)

#define L1_DESC_TYPE_SECTION        0x2u
#define L1_SECTION_BASE_MASK        0xFFF00000u
#define L1_SECTION_DOMAIN_SHIFT     5u
#define L1_SECTION_XN_BIT           (1u << 4)
#define L1_SECTION_AP01_SHIFT       10u
#define L1_SECTION_AP2_BIT          (1u << 15)
#define L1_SECTION_PXN_BIT          (1u << 0)

#define L1_DESC_TYPE_PAGE_TABLE     0x1u
#define L1_PAGE_TABLE_BASE_MASK     0xFFFFFC00u
#define L1_PAGE_TABLE_DOMAIN_SHIFT  5u

#define L2_DESC_TYPE_SMALL_PAGE     0x2u
#define L2_SMALL_PAGE_BASE_MASK     0xFFFFF000u
#define L2_SMALL_PAGE_XN_BIT        (1u << 0)
#define L2_SMALL_PAGE_AP01_SHIFT    4u
#define L2_SMALL_PAGE_AP2_BIT       (1u << 9)

#define THREAD_STACK_GUARD_LOWER_IDX  0
#define THREAD_STACK_PAGE_IDX         1
#define THREAD_STACK_GUARD_UPPER_IDX  2

#define MMU_DOMAIN_KERNEL           0u
#define DACR_DOMAIN_MODE_NO_ACCESS  0x0u
#define DACR_DOMAIN_MODE_CLIENT     0x1u
#define DACR_DOMAIN_MODE_MANAGER    0x3u
#define DACR_DOMAIN_MODE(domain, mode)   ((uint32_t)(mode) << (2u * (uint32_t)(domain)))

#define SCTLR_MMU_ENABLE_BIT        (1u << 0)

static l1_entry l1_page_table[MMU_L1_ENTRIES] __attribute__((aligned(MMU_L1_TABLE_ALIGNMENT)));

typedef struct l2_table_aligned {
    l2_entry e[MMU_L2_ENTRIES];
} __attribute__((aligned(MMU_L2_TABLE_ALIGNMENT))) l2_table_t;

static l2_table_t l2_stack_tables[THREADS_MAX_COUNT];
static l2_table_t l2_boot_region;

#define KERNEL_GUARD_L2_MAX_SECTIONS  4u

static l2_table_t l2_kernel_guard_tables[KERNEL_GUARD_L2_MAX_SECTIONS];
static uint32_t   l2_kernel_guard_sec_base[KERNEL_GUARD_L2_MAX_SECTIONS];
static uint32_t   l2_kernel_guard_sec_count = 0u;

static inline bool is_aligned_uint(uint32_t addr, uint32_t alignment_bytes) {
    return (addr & MMU_ALIGN_MASK(alignment_bytes)) == 0u;
}

static inline uint32_t encode_ap_bits(enum mmu_permission perm, uint32_t ap01_shift, uint32_t ap2_bit) {
    const uint32_t ap = (uint32_t)perm;
    uint32_t bits = 0u;
    
    bits |= ((ap & 0x3u) << ap01_shift);
    if ((ap & 0x4u) != 0u) {
        bits |= ap2_bit;
    }
    
    return bits;
}

static inline uint32_t section_ap_bits(enum mmu_permission perm) {
    return encode_ap_bits(perm, L1_SECTION_AP01_SHIFT, L1_SECTION_AP2_BIT);
}

static inline uint32_t small_page_ap_bits(enum mmu_permission perm) {
    return encode_ap_bits(perm, L2_SMALL_PAGE_AP01_SHIFT, L2_SMALL_PAGE_AP2_BIT);
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

static void synchronize_mmu_state(void) {
    barrier_dsb();
    tlb_invalidate_all();
    barrier_dsb();
    barrier_isb();
}

static void configure_mmu_registers(void) {
    cp15_write_ttbcr(0u);
    cp15_write_ttbr0((uint32_t)(uintptr_t)l1_page_table);
    cp15_write_dacr(DACR_DOMAIN_MODE(MMU_DOMAIN_KERNEL, DACR_DOMAIN_MODE_CLIENT));
}

l2_entry mmu_l2_fault(void) {
    return 0u;
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

l1_entry mmu_l1_page_table(void *l2_table) {
    const uint32_t addr = (uint32_t)(uintptr_t)l2_table;

    if (!is_aligned_uint(addr, MMU_L2_TABLE_ALIGNMENT)) {
        panic("MMU: L2 table not 1KiB aligned");
    }

    const uint32_t domain = MMU_DOMAIN_KERNEL;

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

l1_entry mmu_l1_section(void *phy_addr, enum mmu_permission perm, bool xn, bool pxn) {
    const uint32_t pa = (uint32_t)(uintptr_t)phy_addr;

    if (!is_aligned_uint(pa, MMU_SECTION_SIZE)) {
        panic("MMU: phys addr not 1MiB aligned");
    }

    const uint32_t domain = MMU_DOMAIN_KERNEL;

    uint32_t desc = 0u;
    desc |= (pa & L1_SECTION_BASE_MASK);
    desc |= L1_DESC_TYPE_SECTION;
    desc |= ((domain & 0xFu) << L1_SECTION_DOMAIN_SHIFT);
    desc |= section_ap_bits(perm);

    if (xn) {
        desc |= L1_SECTION_XN_BIT;
    }

    if (pxn) {
        desc |= L1_SECTION_PXN_BIT;
    }

    return desc;
}

static void l1_fill_fault(void) {
    for (uint32_t i = 0; i < MMU_L1_ENTRIES; i++) {
        l1_page_table[i] = mmu_l1_fault();
    }
}

static void map_identity_range(uint32_t start_addr, uint32_t size_bytes,
                               enum mmu_permission perm, bool xn, bool pxn)
{
    const uint32_t first_mb = start_addr >> MMU_SECTION_SHIFT;
    const uint32_t last_addr = start_addr + size_bytes;
    const uint32_t last_mb_exclusive = (last_addr + MMU_ALIGN_MASK(MMU_SECTION_SIZE)) >> MMU_SECTION_SHIFT;

    for (uint32_t mb = first_mb; mb < last_mb_exclusive; mb++) {
        const uint32_t base = mb << MMU_SECTION_SHIFT;
        mmu_set_l1_entry((void *)(uintptr_t)base,
                         mmu_l1_section((void *)(uintptr_t)base, perm, xn, pxn));
    }
}

static l2_table_t *kernel_guard_get_or_init_table(uint32_t sec_base_1mb)
{
    for (uint32_t i = 0; i < l2_kernel_guard_sec_count; i++) {
        if (l2_kernel_guard_sec_base[i] == sec_base_1mb) {
            return &l2_kernel_guard_tables[i];
        }
    }

    if (l2_kernel_guard_sec_count >= KERNEL_GUARD_L2_MAX_SECTIONS) {
        panic("MMU: too many kernel guard-page sections");
    }

    const uint32_t idx = l2_kernel_guard_sec_count++;
    l2_kernel_guard_sec_base[idx] = sec_base_1mb;

    for (uint32_t i = 0; i < MMU_L2_ENTRIES; i++) {
        const uint32_t pa = sec_base_1mb + (i * MMU_SMALL_PAGE_SIZE);
        l2_kernel_guard_tables[idx].e[i] =
            mmu_l2_small_page((void *)(uintptr_t)pa, PERM_RW_NA, true);
    }

    mmu_set_l1_entry((void *)(uintptr_t)sec_base_1mb,
                     mmu_l1_page_table(l2_kernel_guard_tables[idx].e));

    return &l2_kernel_guard_tables[idx];
}

static void kernel_guard_unmap_page(void *page_addr_any)
{
    uint32_t va = (uint32_t)(uintptr_t)page_addr_any;
    va &= ~MMU_ALIGN_MASK(MMU_SMALL_PAGE_SIZE);

    const uint32_t sec_base = (va & L1_SECTION_BASE_MASK);
    l2_table_t *t = kernel_guard_get_or_init_table(sec_base);

    const uint32_t l2_idx = (va >> MMU_SMALL_PAGE_SHIFT) & (MMU_L2_ENTRIES - 1u);
    t->e[l2_idx] = mmu_l2_fault();
}

static void setup_guard_page(unsigned char *guard_page) {
    kernel_guard_unmap_page(guard_page);
}

static void setup_kernel_stack_guards(void) {
    l2_kernel_guard_sec_count = 0u;
    
    setup_guard_page(svc_stack_guard_lower);
    setup_guard_page(svc_stack_guard_upper);
    setup_guard_page(irq_stack_guard_lower);
    setup_guard_page(irq_stack_guard_upper);
    setup_guard_page(abt_stack_guard_lower);
    setup_guard_page(abt_stack_guard_upper);
    setup_guard_page(und_stack_guard_lower);
    setup_guard_page(und_stack_guard_upper);
    setup_guard_page(sys_stack_guard_lower);
    setup_guard_page(sys_stack_guard_upper);
}

static void setup_boot_region(void) {
    for (unsigned int i = 0; i < MMU_L2_ENTRIES; i++) {
        l2_boot_region.e[i] = mmu_l2_fault();
    }

    const uint32_t init_start = (uint32_t)(uintptr_t)&ld_section_init_start;
    const uint32_t init_end = (uint32_t)(uintptr_t)&ld_section_init_end;
    const uint32_t init_end_aligned = (init_end + MMU_ALIGN_MASK(MMU_SMALL_PAGE_SIZE)) 
                                      & ~MMU_ALIGN_MASK(MMU_SMALL_PAGE_SIZE);

    for (uint32_t addr = init_start; addr < init_end_aligned; addr += MMU_SMALL_PAGE_SIZE) {
        const uint32_t idx = (addr >> MMU_SMALL_PAGE_SHIFT) & (MMU_L2_ENTRIES - 1);
        l2_boot_region.e[idx] = mmu_l2_small_page((void *)(uintptr_t)addr,
                                                  PERM_RW_NA,
                                                  false);
    }

    mmu_set_l1_entry((void *)0, mmu_l1_page_table(l2_boot_region.e));
}

static void setup_kernel_sections(void) {
    const uint32_t kernel_text_start = (uint32_t)(uintptr_t)&ld_section_kernel_text;
    const uint32_t kernel_text_end = (uint32_t)(uintptr_t)&ld_section_kernel_text_end;
    const uint32_t kernel_rodata_start = (uint32_t)(uintptr_t)&ld_section_kernel_rodata;
    const uint32_t kernel_rodata_end = (uint32_t)(uintptr_t)&ld_section_kernel_rodata_end;
    const uint32_t kernel_data_bss_start = (uint32_t)(uintptr_t)&ld_section_kernel_data_bss;
    const uint32_t kernel_data_bss_end = (uint32_t)(uintptr_t)&ld_section_kernel_data_bss_end;

    map_identity_range(kernel_text_start, kernel_text_end - kernel_text_start,
                      PERM_R_NA, false, false);
    map_identity_range(kernel_rodata_start, kernel_rodata_end - kernel_rodata_start,
                      PERM_R_NA, true, false);
    map_identity_range(kernel_data_bss_start, kernel_data_bss_end - kernel_data_bss_start,
                      PERM_RW_NA, true, false);
}

static void setup_user_sections(void) {
    const uint32_t user_text_start = (uint32_t)(uintptr_t)&ld_section_user_text;
    const uint32_t user_text_end = (uint32_t)(uintptr_t)&ld_section_user_text_end;
    const uint32_t user_rodata_start = (uint32_t)(uintptr_t)&ld_section_user_rodata;
    const uint32_t user_rodata_end = (uint32_t)(uintptr_t)&ld_section_user_rodata_end;
    const uint32_t user_data_bss_start = (uint32_t)(uintptr_t)&ld_section_user_data_bss;
    const uint32_t user_data_bss_end = (uint32_t)(uintptr_t)&ld_section_user_data_bss_end;

    map_identity_range(user_text_start, user_text_end - user_text_start,
                      PERM_R_R, false, true);
    map_identity_range(user_rodata_start, user_rodata_end - user_rodata_start,
                      PERM_R_R, true, false);
    map_identity_range(user_data_bss_start, user_data_bss_end - user_data_bss_start,
                      PERM_FULL_ACCESS, true, false);
}

static void setup_thread_stacks(void) {
    for (uint32_t tid = 0; tid < THREADS_MAX_COUNT; tid++) {
        const uint32_t win_base = THREADS_STACK_WIN_BASE(tid);

        mmu_set_l1_entry((void *)(uintptr_t)win_base,
                         mmu_l1_page_table(l2_stack_tables[tid].e));
        
        for (uint32_t i = 0; i < MMU_L2_ENTRIES; i++) {
            l2_stack_tables[tid].e[i] = mmu_l2_fault();
        }

        l2_stack_tables[tid].e[THREAD_STACK_PAGE_IDX] = mmu_l2_small_page(&thread_stacks_phys[tid][0],
                                                                         PERM_FULL_ACCESS,
                                                                         true);
        l2_stack_tables[tid].e[THREAD_STACK_GUARD_UPPER_IDX] = mmu_l2_fault();
    }
}

void mmu_init(void) {
    l1_fill_fault();

    for (uint32_t mb = 0; mb < MMU_L1_ENTRIES; mb++) {
        const uint32_t base = mb << MMU_SECTION_SHIFT;
        mmu_set_l1_entry((void *)(uintptr_t)base,
                         mmu_l1_section((void *)(uintptr_t)base,
                                       PERM_FULL_ACCESS, false, false));
    }

    configure_mmu_registers();
    synchronize_mmu_state();

    check_mmu_1_to_1(l1_page_table);
}

void mmu_setup_protection(void) {
    l1_fill_fault();

    setup_boot_region();
    setup_kernel_sections();
    setup_kernel_stack_guards();
    setup_user_sections();

    map_identity_range(MMU_PERIPH_BASE, MMU_PERIPH_SIZE_BYTES,
                      PERM_RW_NA, true, false);

    setup_thread_stacks();

    synchronize_mmu_state();
}

void mmu_enable(void) {
    configure_mmu_registers();
    synchronize_mmu_state();

    uint32_t sctlr = cp15_read_sctlr();
    sctlr |= SCTLR_MMU_ENABLE_BIT;
    cp15_write_sctlr(sctlr);
    barrier_isb();
}
