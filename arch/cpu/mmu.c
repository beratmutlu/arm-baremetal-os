#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <arch/cpu/mmu.h>
#include <kernel/panic.h>
#include <kernel/diagnose_mmu.h>
#include <kernel/threads.h>

extern unsigned char thread_stacks_phys[THREADS_MAX_COUNT][THREADS_STACK_PAGE_SIZE];

extern volatile unsigned char svc_stack_guard_lower[];
extern volatile unsigned char svc_stack[];
extern volatile unsigned char svc_stack_guard_upper[];

extern volatile unsigned char irq_stack_guard_lower[];
extern volatile unsigned char irq_stack[];
extern volatile unsigned char irq_stack_guard_upper[];

extern volatile unsigned char abt_stack_guard_lower[];
extern volatile unsigned char abt_stack[];
extern volatile unsigned char abt_stack_guard_upper[];

extern volatile unsigned char und_stack_guard_lower[];
extern volatile unsigned char und_stack[];
extern volatile unsigned char und_stack_guard_upper[];

extern volatile unsigned char sys_stack_guard_lower[];
extern volatile unsigned char sys_stack[];
extern volatile unsigned char sys_stack_guard_upper[];

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

#define MMU_USER_DATA_TEMPLATE_WIN_BASE 0x00800000u
#define MMU_USER_DATA_SCRATCH_WIN_BASE  0x00900000u

/**
 * Address spaces use a single shared L1; user .data/.bss is mapped via per-AS L2 tables.
 * New address spaces are initialized by copying from a template window into a scratch window.
 * TLBs are fully invalidated on AS switch; caches remain disabled.
 */
typedef struct mmu_as {
    bool in_use;
    uint32_t refcount;
    uint32_t data_bss_phys_base;
    l2_table_t data_l2;
} mmu_as_t;

static mmu_as_t   as_table[MMU_AS_MAX_COUNT];
static l2_table_t l2_user_data_template;
static l2_table_t l2_user_data_scratch;
static mmu_asid_t current_asid = MMU_ASID_INVALID;
static uint32_t   user_data_bss_start = 0u;
static uint32_t   user_data_bss_end = 0u;
static uint32_t   user_data_bss_size = 0u;
static uint32_t   user_data_bss_pages = 0u;
static uint32_t   user_data_section_base = 0u;
static uint32_t   user_data_section_offset = 0u;
static uint32_t   as_phys_next = 0u;

#define KERNEL_GUARD_L2_MAX_SECTIONS  4u

static l2_table_t l2_kernel_guard_tables[KERNEL_GUARD_L2_MAX_SECTIONS];
static uint32_t   l2_kernel_guard_sec_base[KERNEL_GUARD_L2_MAX_SECTIONS];
static uint32_t   l2_kernel_guard_sec_count = 0u;

static inline bool is_aligned_uint(uint32_t addr, uint32_t alignment_bytes) {
    return (addr & MMU_ALIGN_MASK(alignment_bytes)) == 0u;
}

static inline uint32_t align_up_uint(uint32_t value, uint32_t alignment_bytes) {
    return (value + MMU_ALIGN_MASK(alignment_bytes)) & ~MMU_ALIGN_MASK(alignment_bytes);
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

static void l2_fill_fault(l2_table_t *t) {
    for (uint32_t i = 0; i < MMU_L2_ENTRIES; i++) {
        t->e[i] = mmu_l2_fault();
    }
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

static void setup_guard_page(volatile unsigned char *guard_page) {
    kernel_guard_unmap_page((void *)guard_page);
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
                                                  PERM_R_NA,
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
                      PERM_R_NA, true, true);
    map_identity_range(kernel_data_bss_start, kernel_data_bss_end - kernel_data_bss_start,
                      PERM_RW_NA, true, true);
}

static void setup_user_sections(void) {
    const uint32_t user_text_start = (uint32_t)(uintptr_t)&ld_section_user_text;
    const uint32_t user_text_end = (uint32_t)(uintptr_t)&ld_section_user_text_end;
    const uint32_t user_rodata_start = (uint32_t)(uintptr_t)&ld_section_user_rodata;
    const uint32_t user_rodata_end = (uint32_t)(uintptr_t)&ld_section_user_rodata_end;

    map_identity_range(user_text_start, user_text_end - user_text_start,
                      PERM_R_R, false, true);
    map_identity_range(user_rodata_start, user_rodata_end - user_rodata_start,
                      PERM_R_R, true, true);
}

static void setup_user_data_as_support(void) {
    user_data_bss_start = (uint32_t)(uintptr_t)&ld_section_user_data_bss;
    user_data_bss_end = (uint32_t)(uintptr_t)&ld_section_user_data_bss_end;

    if (user_data_bss_end < user_data_bss_start) {
        panic("MMU: user data/bss bounds invalid");
    }
    if (!is_aligned_uint(user_data_bss_start, MMU_SMALL_PAGE_SIZE)) {
        panic("MMU: user data/bss not 4KiB aligned");
    }
    if (!is_aligned_uint(MMU_USER_DATA_TEMPLATE_WIN_BASE, MMU_SECTION_SIZE) ||
        !is_aligned_uint(MMU_USER_DATA_SCRATCH_WIN_BASE, MMU_SECTION_SIZE)) {
        panic("MMU: user data windows not 1MiB aligned");
    }

    user_data_bss_size = user_data_bss_end - user_data_bss_start;
    user_data_bss_pages = (user_data_bss_size + MMU_ALIGN_MASK(MMU_SMALL_PAGE_SIZE)) >> MMU_SMALL_PAGE_SHIFT;
    user_data_section_base = user_data_bss_start & ~MMU_ALIGN_MASK(MMU_SECTION_SIZE);
    user_data_section_offset = user_data_bss_start - user_data_section_base;

    if ((user_data_section_offset + user_data_bss_size) > MMU_SECTION_SIZE) {
        panic("MMU: user data/bss exceeds 1MiB section");
    }
    if (((user_data_section_offset >> MMU_SMALL_PAGE_SHIFT) + user_data_bss_pages) > MMU_L2_ENTRIES) {
        panic("MMU: user data/bss too large for L2");
    }

    for (uint32_t i = 0; i < MMU_AS_MAX_COUNT; i++) {
        as_table[i].in_use = false;
        as_table[i].refcount = 0u;
        as_table[i].data_bss_phys_base = 0u;
        l2_fill_fault(&as_table[i].data_l2);
    }

    l2_fill_fault(&l2_user_data_template);
    l2_fill_fault(&l2_user_data_scratch);

    const uint32_t first_idx = user_data_section_offset >> MMU_SMALL_PAGE_SHIFT;
    for (uint32_t i = 0; i < user_data_bss_pages; i++) {
        const uint32_t pa = user_data_bss_start + (i * MMU_SMALL_PAGE_SIZE);
        l2_user_data_template.e[first_idx + i] =
            mmu_l2_small_page((void *)(uintptr_t)pa, PERM_R_NA, true);
    }

    mmu_set_l1_entry((void *)(uintptr_t)MMU_USER_DATA_TEMPLATE_WIN_BASE,
                     mmu_l1_page_table(l2_user_data_template.e));
    mmu_set_l1_entry((void *)(uintptr_t)MMU_USER_DATA_SCRATCH_WIN_BASE,
                     mmu_l1_page_table(l2_user_data_scratch.e));

    mmu_set_l1_entry((void *)(uintptr_t)user_data_section_base, mmu_l1_fault());

    as_phys_next = align_up_uint(user_data_bss_end, MMU_SMALL_PAGE_SIZE);
    current_asid = MMU_ASID_INVALID;
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

mmu_asid_t mmu_as_create(void) {
    mmu_asid_t free_asid = MMU_ASID_INVALID;
    const uint32_t size_aligned = align_up_uint(user_data_bss_size, MMU_SMALL_PAGE_SIZE);

    for (uint32_t i = 0; i < MMU_AS_MAX_COUNT; i++) {
        if (!as_table[i].in_use) {
            free_asid = (mmu_asid_t)i;
            break;
        }
    }

    if (free_asid == MMU_ASID_INVALID) {
        return MMU_ASID_INVALID;
    }

    mmu_as_t *as = &as_table[free_asid];

    if (as->data_bss_phys_base == 0u) {
        const uint64_t end = (uint64_t)as_phys_next + (uint64_t)size_aligned;
        if (end > MMU_USABLE_RAM_SIZE_BYTES) {
            return MMU_ASID_INVALID;
        }
        as->data_bss_phys_base = as_phys_next;
        as_phys_next += size_aligned;
    }

    as->in_use = true;
    as->refcount = 1u;

    l2_fill_fault(&as->data_l2);
    const uint32_t first_idx = user_data_section_offset >> MMU_SMALL_PAGE_SHIFT;
    for (uint32_t i = 0; i < user_data_bss_pages; i++) {
        const uint32_t pa = as->data_bss_phys_base + (i * MMU_SMALL_PAGE_SIZE);
        as->data_l2.e[first_idx + i] =
            mmu_l2_small_page((void *)(uintptr_t)pa, PERM_FULL_ACCESS, true);
    }

    l2_fill_fault(&l2_user_data_scratch);
    for (uint32_t i = 0; i < user_data_bss_pages; i++) {
        const uint32_t pa = as->data_bss_phys_base + (i * MMU_SMALL_PAGE_SIZE);
        l2_user_data_scratch.e[first_idx + i] =
            mmu_l2_small_page((void *)(uintptr_t)pa, PERM_RW_NA, true);
    }

    synchronize_mmu_state();

    if (size_aligned > 0u) {
        uint8_t *dst = (uint8_t *)(uintptr_t)(MMU_USER_DATA_SCRATCH_WIN_BASE + user_data_section_offset);
        const uint8_t *src = (const uint8_t *)(uintptr_t)(MMU_USER_DATA_TEMPLATE_WIN_BASE + user_data_section_offset);
        if (user_data_bss_size > 0u) {
            memcpy(dst, src, user_data_bss_size);
        }
        if (size_aligned > user_data_bss_size) {
            memset(dst + user_data_bss_size, 0, size_aligned - user_data_bss_size);
        }
    }

    return free_asid;
}

void mmu_as_retain(mmu_asid_t asid) {
    if (asid >= MMU_AS_MAX_COUNT) {
        return;
    }
    if (!as_table[asid].in_use) {
        return;
    }
    as_table[asid].refcount++;
}

void mmu_as_release(mmu_asid_t asid) {
    if (asid >= MMU_AS_MAX_COUNT) {
        return;
    }
    mmu_as_t *as = &as_table[asid];
    if (!as->in_use) {
        return;
    }
    if (as->refcount > 0u) {
        as->refcount--;
    }
    if (as->refcount == 0u) {
        as->in_use = false;
        if (current_asid == asid) {
            current_asid = MMU_ASID_INVALID;
        }
    }
}

void mmu_switch_as(mmu_asid_t asid) {
    if (asid >= MMU_AS_MAX_COUNT) {
        return;
    }
    if (!as_table[asid].in_use) {
        return;
    }
    if (asid == current_asid) {
        return;
    }

    mmu_set_l1_entry((void *)(uintptr_t)user_data_section_base,
                     mmu_l1_page_table(as_table[asid].data_l2.e));
    current_asid = asid;
    synchronize_mmu_state();
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
    setup_user_data_as_support();

    map_identity_range(MMU_PERIPH_BASE, MMU_PERIPH_SIZE_BYTES,
                      PERM_RW_NA, true, true);

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
