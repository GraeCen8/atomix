#include "paging.h"
#include "pmm.h"
#include "../util.h"

#define PAGE_ALIGN_DOWN(x) ((x) & ~(PAGING_PAGE_SIZE - 1u))
#define PAGE_ALIGN_UP(x) (((x) + (PAGING_PAGE_SIZE - 1u)) & ~(PAGING_PAGE_SIZE - 1u))
#define PDE_INDEX(x) (((x) >> 22) & 0x3FFu)
#define PTE_INDEX(x) (((x) >> 12) & 0x3FFu)
#define PAGE_OFFSET(x) ((x) & 0xFFFu)

static uint32_t *g_page_directory;
static uint32_t g_mapped_pages;
static int g_enabled;

static void load_page_directory(uint32_t phys_addr) {
  asm volatile("mov %0, %%cr3" : : "r"(phys_addr) : "memory");
}

static void enable_paging_bit(void) {
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000u;
  asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

static void invlpg(uint32_t virt_addr) {
  asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

static uint32_t *alloc_page_table(void) {
  uint32_t frame = pmm_alloc_frame();
  if (frame == 0) {
    return 0;
  }

  uint32_t *table = (uint32_t *)(uintptr_t)frame;
  memset(table, 0, PAGING_PAGE_SIZE);
  return table;
}

static int map_internal(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags,
                        int flush_tlb) {
  if (g_page_directory == 0) {
    return 0;
  }

  uint32_t pde = PDE_INDEX(virt_addr);
  uint32_t pte = PTE_INDEX(virt_addr);
  uint32_t effective_flags = flags & 0xFFFu;

  if ((g_page_directory[pde] & PAGING_FLAG_PRESENT) == 0) {
    uint32_t *new_table = alloc_page_table();
    if (new_table == 0) {
      return 0;
    }

    g_page_directory[pde] = ((uint32_t)(uintptr_t)new_table & 0xFFFFF000u) |
                            PAGING_FLAG_PRESENT | PAGING_FLAG_WRITABLE;
  }

  uint32_t *table =
      (uint32_t *)(uintptr_t)(g_page_directory[pde] & 0xFFFFF000u);

  if ((table[pte] & PAGING_FLAG_PRESENT) == 0) {
    g_mapped_pages++;
  }

  table[pte] = (phys_addr & 0xFFFFF000u) | effective_flags | PAGING_FLAG_PRESENT;

  if (flush_tlb && g_enabled) {
    invlpg(PAGE_ALIGN_DOWN(virt_addr));
  }

  return 1;
}

int paging_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
  return map_internal(PAGE_ALIGN_DOWN(virt_addr), PAGE_ALIGN_DOWN(phys_addr),
                      flags, 1);
}

int paging_unmap(uint32_t virt_addr) {
  if (g_page_directory == 0) {
    return 0;
  }

  uint32_t aligned = PAGE_ALIGN_DOWN(virt_addr);
  uint32_t pde = PDE_INDEX(aligned);
  uint32_t pte = PTE_INDEX(aligned);

  if ((g_page_directory[pde] & PAGING_FLAG_PRESENT) == 0) {
    return 0;
  }

  uint32_t *table =
      (uint32_t *)(uintptr_t)(g_page_directory[pde] & 0xFFFFF000u);
  if ((table[pte] & PAGING_FLAG_PRESENT) == 0) {
    return 0;
  }

  table[pte] = 0;
  if (g_mapped_pages > 0) {
    g_mapped_pages--;
  }

  if (g_enabled) {
    invlpg(aligned);
  }

  return 1;
}

int paging_translate(uint32_t virt_addr, uint32_t *phys_out) {
  if (phys_out == 0 || g_page_directory == 0) {
    return 0;
  }

  uint32_t pde = PDE_INDEX(virt_addr);
  uint32_t pte = PTE_INDEX(virt_addr);

  if ((g_page_directory[pde] & PAGING_FLAG_PRESENT) == 0) {
    return 0;
  }

  uint32_t *table =
      (uint32_t *)(uintptr_t)(g_page_directory[pde] & 0xFFFFF000u);
  uint32_t entry = table[pte];
  if ((entry & PAGING_FLAG_PRESENT) == 0) {
    return 0;
  }

  *phys_out = (entry & 0xFFFFF000u) | PAGE_OFFSET(virt_addr);
  return 1;
}

void paging_init(uint32_t kernel_end, uint32_t heap_start, uint32_t heap_size) {
  uint32_t dir_frame = pmm_alloc_frame();
  if (dir_frame == 0) {
    return;
  }

  g_page_directory = (uint32_t *)(uintptr_t)dir_frame;
  memset(g_page_directory, 0, PAGING_PAGE_SIZE);
  g_mapped_pages = 0;

  uint32_t identity_end = kernel_end;
  uint32_t heap_end = heap_start + heap_size;
  if (heap_end > identity_end) {
    identity_end = heap_end;
  }

  identity_end = PAGE_ALIGN_UP(identity_end);
  for (uint32_t addr = 0; addr < identity_end; addr += PAGING_PAGE_SIZE) {
    if (!map_internal(addr, addr, PAGING_FLAG_WRITABLE, 0)) {
      return;
    }
  }

  load_page_directory(dir_frame);
  enable_paging_bit();
  g_enabled = 1;
}

int paging_enabled(void) { return g_enabled; }

uint32_t paging_mapped_pages(void) { return g_mapped_pages; }
