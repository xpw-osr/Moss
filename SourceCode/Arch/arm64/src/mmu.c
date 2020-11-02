#include "arm64/mmu.h"
#include "libc/log.h"
#include "libc/types.h"

#define TTBR_CNP 1

typedef unsigned long int uint64_t;

/// main table
static unsigned long main_tbl[512 * 20] __attribute__((aligned(4096)));

#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a)-1)) == 0)

#define PMD_TYPE_SECT (1 << 0)

#define PMD_TYPE_TABLE (3 << 0)

#define PTE_TYPE_PAGE (3 << 0)

#define BITS_PER_VA 39

/* Granule size of 4KB is being used */
#define GRANULE_SIZE_SHIFT 12
#define GRANULE_SIZE (1 << GRANULE_SIZE_SHIFT)
#define XLAT_ADDR_MASK ((1UL << BITS_PER_VA) - GRANULE_SIZE)

#define PMD_TYPE_MASK (3 << 0)

/*
 * Memory types available.
 */
#define MT_DEVICE_nGnRnE 0
#define MT_DEVICE_nGnRE 1
#define MT_DEVICE_GRE 2
#define MT_NORMAL_NC 3
#define MT_NORMAL 4
#define MT_NORMAL_WT 5

#define MAIR(attr, mt) ((attr) << ((mt)*8))

int free_idx = 1;

void __asm_invalidate_icache_all(void);
void __asm_flush_dcache_all(void);
int __asm_flush_l3_cache(void);
void __asm_flush_dcache_range(unsigned long long start, unsigned long long end);
void __asm_invalidate_dcache_all(void);
void __asm_invalidate_icache_all(void);

void mmu_memset(char *dst, char v, usize len) {
  while (len--) {
    *dst++ = v;
  }
}

static unsigned long __page_off = 0;
static unsigned long get_free_page(void) {
  __page_off += 512;
  return (unsigned long)(main_tbl + __page_off);
}

static inline unsigned int get_sctlr(void) {
  unsigned int val;
  asm volatile("mrs %0, sctlr_el1" : "=r"(val) : : "cc");
  return val;
}

static inline void set_sctlr(unsigned int val) {
  asm volatile("msr sctlr_el1, %0" : : "r"(val) : "cc");
  asm volatile("isb");
}

void mmu_init(void) {
//  u64 mair = mair = MAIR(0x00UL, MT_DEVICE_nGnRnE) | MAIR(0x04UL, MT_DEVICE_nGnRE) | MAIR(0x0cUL, MT_DEVICE_GRE) |
//                    MAIR(0x44UL, MT_NORMAL_NC) | MAIR(0xffUL, MT_NORMAL) | MAIR(0xbbUL, MT_NORMAL_WT);
//  log_i("mair: 0x%llx", mair);
//  u64 mair_el1 = mair; // also woks? why?
  u64 mair_el1 = 0x007f6eUL; // 0b111_1111_0110_1110
  /// Memory Attribute Indirection Register
  asm volatile("msr MAIR_EL1, %0\ndsb sy\n" ::"r"(mair_el1));
  asm volatile("mrs %0, MAIR_EL1\ndsb sy\n" : "=r"(mair_el1));
  log_i("mair_el1: 0x%llx", mair_el1);

  // TCR_EL1
  u64 tcr_el1 = (16UL << 0)                                                  // 48bit
                | (0x0UL << 6) | (0x0UL << 7) | (0x3UL << 8) | (0x3UL << 10) // Inner Shareable
                | (0x2UL << 12) | (0x0UL << 14)                              // 4K
                | (0x0UL << 16) | (0x0UL << 22) | (0x1UL << 23) | (0x2UL << 30) | (0x1UL << 32) | (0x0UL << 35) |
                (0x0UL << 36) | (0x0UL << 37) | (0x0UL << 38);

  asm volatile("msr TCR_EL1, %0\n" ::"r"(tcr_el1));
  asm volatile("mrs %0, TCR_EL1\n" : "=r"(tcr_el1));

  asm volatile("msr TTBR0_EL1, %0\ndsb sy\n" ::"r"(main_tbl));
  asm volatile("mrs %0, TTBR0_EL1\ndsb sy\n" : "=r"(mair_el1));

  mmu_memset((char *)main_tbl, 0, 4096);
}

void mmu_enable(void) {
  unsigned long sctlr_el1 = 0;
  asm volatile("mrs %0, SCTLR_EL1\n" : "=r"(sctlr_el1));
  sctlr_el1 &= ~0x1000; // disable I
  asm volatile("dmb sy\n msr SCTLR_EL1, %0\n isb sy\n" ::"r"(sctlr_el1));

  asm volatile("IC IALLUIS\n dsb sy\n isb sy\n");
  asm volatile("tlbi vmalle1\n dsb sy\n isb sy\n");

  unsigned long val32 = 0;
  // SCTLR_EL1, turn on mmu
  asm volatile("mrs %0, SCTLR_EL1\n" : "=r"(val32));
  val32 |= 0x1005; // enable mmu, I C M
  asm volatile("dmb sy\n msr SCTLR_EL1, %0\nisb sy\n" ::"r"(val32));
  rt_hw_icache_enable();
  rt_hw_dcache_enable();
}

static int map_single_page_2M(unsigned long *lv0_tbl, unsigned long va, unsigned long pa, unsigned long attr) {
  int level;
  unsigned long *cur_lv_tbl = lv0_tbl;
  unsigned long page;
  unsigned long off;
  int level_shift = 39;

  if (va & (0x200000UL - 1)) {
    return MMU_MAP_ERROR_VANOTALIGN;
  }
  if (pa & (0x200000UL - 1)) {
    return MMU_MAP_ERROR_PANOTALIGN;
  }
  for (level = 0; level < 2; level++) {
    off = (va >> level_shift);
    off &= MMU_LEVEL_MASK;
    if ((cur_lv_tbl[off] & 1) == 0) {
      page = get_free_page();
      if (!page) {
        return MMU_MAP_ERROR_NOPAGE;
      }
      mmu_memset((char *)page, 0, 4096);
      cur_lv_tbl[off] = page | 0x3UL;
    }
    page = cur_lv_tbl[off];
    if (!(page & 0x2)) {
      // is block! error!
      return MMU_MAP_ERROR_CONFLICT;
    }
    cur_lv_tbl = (unsigned long *)(page & 0x0000fffffffff000UL);
    level_shift -= 9;
  }
  attr &= 0xfff0000000000ffcUL;
  pa |= (attr | 0x1UL); // block
  off = (va >> 21);
  off &= MMU_LEVEL_MASK;
  cur_lv_tbl[off] = pa;
  return 0;
}

int armv8_map_2M(unsigned long va, unsigned long pa, int count, unsigned long attr) {
  int i;
  int ret;

  if (va & (0x200000 - 1)) {
    return -1;
  }
  if (pa & (0x200000 - 1)) {
    return -1;
  }
  for (i = 0; i < count; i++) {
    ret = map_single_page_2M((unsigned long *)main_tbl, va, pa, attr);
    va += 0x200000;
    pa += 0x200000;
    if (ret != 0) {
      return ret;
    }
  }
  return 0;
}

static void set_table(uint64_t *pt, uint64_t *table_addr) {
  uint64_t val;
  val = (0x3UL | (uint64_t)table_addr);
  *pt = val;
}

void mmu_memset2(unsigned char *dst, char v, int len) {
  while (len--) {
    *dst++ = v;
  }
}

static uint64_t *create_table(void) {
  uint64_t *new_table = (uint64_t *)((unsigned char *)&main_tbl[0] + free_idx * 4096); //+ free_idx * GRANULE_SIZE;
  /* Mark all entries as invalid */
  mmu_memset2((unsigned char *)new_table, 0, 4096);
  free_idx++;
  return new_table;
}

static int pte_type(uint64_t *pte) { return *pte & PMD_TYPE_MASK; }

static int level2shift(int level) {
  /* Page is 12 bits wide, every level translates 9 bits */
  return (12 + 9 * (3 - level));
}

static uint64_t *get_level_table(uint64_t *pte) {
  uint64_t *table = (uint64_t *)(*pte & XLAT_ADDR_MASK);

  if (pte_type(pte) != PMD_TYPE_TABLE) {
    table = create_table();
    set_table(pte, table);
  }
  return table;
}

static void map_region(uint64_t virt, uint64_t phys, uint64_t size, uint64_t attr) {
  uint64_t block_size = 0;
  uint64_t block_shift = 0;
  uint64_t *pte;
  uint64_t idx = 0;
  uint64_t addr = 0;
  uint64_t *table = 0;
  int level = 0;

  addr = virt;
  while (size) {
    table = &main_tbl[0];
    //    log_d("table: %p", table);
    for (level = 0; level < 4; level++) {
      block_shift = level2shift(level);
      idx = addr >> block_shift;
      idx = idx % 512;
      block_size = (uint64_t)(1L << block_shift);
      pte = table + idx;

      if (size >= block_size && IS_ALIGNED(addr, block_size)) {
        attr &= 0xfff0000000000ffcUL;
        if (level != 3) {
          *pte = phys | (attr | 0x1UL);
        } else {
          *pte = phys | (attr | 0x3UL);
        }
        addr += block_size;
        phys += block_size;
        size -= block_size;
        break;
      }
      table = get_level_table(pte);
      //      log_d("level table: %p", table);
    }
  }
}

void armv8_map(unsigned long va, unsigned long pa, unsigned long size, unsigned long attr) {
  map_region(va, pa, size, attr);
}

void rt_hw_dcache_enable(void) {
  if (!(get_sctlr() & CR_M)) {
    log_e("please init mmu!\n");
  } else {
    set_sctlr(get_sctlr() | CR_C);
  }
}

void rt_hw_dcache_flush_all(void) {
  int ret;

  __asm_flush_dcache_all();
  ret = __asm_flush_l3_cache();
  if (ret) {
    log_d("flushing dcache returns 0x%x\n", ret);
  } else {
    log_i("flushing dcache successfully.\n");
  }
}

void rt_hw_dcache_flush_range(unsigned long start_addr, unsigned long size) {
  __asm_flush_dcache_range(start_addr, start_addr + size);
}
void rt_hw_dcache_invalidate_range(unsigned long start_addr, unsigned long size) {
  __asm_flush_dcache_range(start_addr, start_addr + size);
}

void rt_hw_dcache_invalidate_all(void) { __asm_invalidate_dcache_all(); }

void rt_hw_dcache_disable(void) {
  /* if cache isn't enabled no need to disable */
  if (!(get_sctlr() & CR_C)) {
    log_e("need enable cache!\n");
    return;
  }
  set_sctlr(get_sctlr() & ~CR_C);
}

// icache
void rt_hw_icache_enable(void) {
  __asm_invalidate_icache_all();
  set_sctlr(get_sctlr() | CR_I);
}

void rt_hw_icache_invalidate_all(void) { __asm_invalidate_icache_all(); }

void rt_hw_icache_disable(void) { set_sctlr(get_sctlr() & ~CR_I); }

extern void paging_init(void);

void init_mmu() {
//  mmu_init();
//  log_d("begin mmu map");
//  armv8_map(0, 0, 0x6400000, MEM_ATTR_MEMORY);
//  armv8_map(0x3f000000, 0x3f000000, 0x200000, MEM_ATTR_IO); // timer
//  armv8_map(0x3f200000, 0x3f200000, 0x16000, MEM_ATTR_IO);  // uart
//  armv8_map(0x40000000, 0x40000000, 0x1000, MEM_ATTR_IO);   // core timer
//  armv8_map(0x3F300000, 0x3F300000, 0x1000, MEM_ATTR_IO);   // sdio
//  armv8_map(0xc00000, 0xc00000, 0x1000, MEM_ATTR_IO);       // mbox
//  armv8_map(0x3f804000, 0x3f804000, 0x1000, MEM_ATTR_IO);   // i2c0
//  armv8_map(0x3f205000, 0x3f205000, 0x1000, MEM_ATTR_IO);   // i2c1
//  log_d("end mmu map");
//  mmu_enable();
  paging_init();
}
