#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "gdt.h"
#include "idt.h"
#include "mem/kmalloc.h"
#include "mem/pmm.h"
#include "shell/shell.h"
#include "timer.h"
#include <stdint.h>

void boot(uint64_t total_memory_bytes);
extern uint8_t __kernel_end;
static uint32_t heap_base;
static uint32_t heap_size;
static uint64_t detected_ram_bytes;

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002u
#define MULTIBOOT_INFO_FLAG_MEM (1u << 0)

typedef struct multiboot_info {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
} multiboot_info_t;

static void terminal_write_u32(uint32_t value) {
  if (value == 0) {
    terminal_putchar('0');
    return;
  }

  uint32_t div = 1000000000u;
  while (div > 0 && (value / div) == 0) {
    div /= 10u;
  }

  while (div > 0) {
    uint32_t digit = value / div;
    terminal_putchar((char)('0' + digit));
    value %= div;
    div /= 10u;
  }
}

void print_mem_stats(void) {
  terminal_write("PMM frames total/used/free: ");
  terminal_write_u32(pmm_total_frames());
  terminal_write("/");
  terminal_write_u32(pmm_used_frames());
  terminal_write("/");
  terminal_write_u32(pmm_free_frames());
  terminal_write("\n");

  terminal_write("RAM detected bytes: ");
  terminal_write_u32((uint32_t)detected_ram_bytes);
  terminal_write(" (low 32 bits), MiB=");
  terminal_write_u32((uint32_t)(detected_ram_bytes / (1024ull * 1024ull)));
  terminal_write("\n");

  if (heap_size > 0) {
    terminal_write("Heap base/size bytes: ");
    terminal_write_u32(heap_base);
    terminal_write("/");
    terminal_write_u32(heap_size);
    terminal_write("\n");
  }
}

static int run_memtest(void) {
  int ok = 1;
  terminal_write("memtest: starting\n");

  uint32_t used_before = pmm_used_frames();
  uint32_t a = pmm_alloc_frame();
  uint32_t b = pmm_alloc_frame();
  if (a == 0 || b == 0 || b != (a + PMM_FRAME_SIZE)) {
    terminal_write("memtest: pmm_alloc_frame failed\n");
    ok = 0;
  }

  uint32_t run = pmm_alloc_contiguous(3);
  if (run == 0) {
    terminal_write("memtest: pmm_alloc_contiguous failed\n");
    ok = 0;
  }

  if (a != 0) {
    pmm_free_frame(a);
  }
  if (b != 0) {
    pmm_free_frame(b);
  }
  if (run != 0) {
    pmm_free_frame(run);
    pmm_free_frame(run + PMM_FRAME_SIZE);
    pmm_free_frame(run + (2u * PMM_FRAME_SIZE));
  }

  if (pmm_used_frames() != used_before) {
    terminal_write("memtest: pmm frame accounting mismatch\n");
    ok = 0;
  }

  void *p1 = kmalloc(64);
  void *p2 = kmalloc(128);
  void *p3 = kmalloc(256);
  if (p1 == 0 || p2 == 0 || p3 == 0) {
    terminal_write("memtest: kmalloc basic allocations failed\n");
    ok = 0;
  }

  kfree(p2);
  void *p2_reuse = kmalloc(96);
  if (p2_reuse == 0) {
    terminal_write("memtest: kmalloc reuse failed\n");
    ok = 0;
  }

  kfree(p1);
  kfree(p3);
  kfree(p2_reuse);
  kmalloc_defrag();

  if (heap_size > 0) {
    void *big = kmalloc((size_t)(heap_size / 2u));
    if (big == 0) {
      terminal_write("memtest: kmalloc coalesce/large alloc failed\n");
      ok = 0;
    } else {
      kfree(big);
    }
  }

  terminal_write("memtest: done\n");
  return ok;
}

static uint64_t detect_total_memory_bytes(uint32_t mb_magic,
                                          uint32_t mb_info_addr) {
  if (mb_magic != MULTIBOOT_BOOTLOADER_MAGIC || mb_info_addr == 0) {
    return PMM_MAX_MEMORY_BYTES;
  }

  const multiboot_info_t *mbi = (const multiboot_info_t *)(uintptr_t)mb_info_addr;
  if ((mbi->flags & MULTIBOOT_INFO_FLAG_MEM) == 0) {
    return PMM_MAX_MEMORY_BYTES;
  }

  uint64_t total = ((uint64_t)mbi->mem_lower + (uint64_t)mbi->mem_upper) * 1024ull;
  if (total == 0 || total > PMM_MAX_MEMORY_BYTES) {
    return PMM_MAX_MEMORY_BYTES;
  }

  return total;
}

void kmain(uint32_t mb_magic, uint32_t mb_info_addr) {
  uint64_t total_memory_bytes = detect_total_memory_bytes(mb_magic, mb_info_addr);
  boot(total_memory_bytes);

  while (1) {
    asm volatile("hlt");
  }
}

void boot(uint64_t total_memory_bytes) {
  const uint32_t heap_size_bytes = 1024u * 1024u;

  terminal_initialize();
  terminal_write("Atomix OS Starting...\n");

  init_gdt();
  init_idt();

  detected_ram_bytes = total_memory_bytes;
  pmm_init(total_memory_bytes, (uint32_t)(uintptr_t)&__kernel_end);
  uint32_t pages = heap_size_bytes / PMM_FRAME_SIZE;
  if ((heap_size_bytes % PMM_FRAME_SIZE) != 0) {
    pages++;
  }

  uint32_t heap_start = pmm_alloc_contiguous(pages);
  if (heap_start != 0) {
    heap_base = heap_start;
    heap_size = pages * PMM_FRAME_SIZE;
    kmalloc_init(heap_start, heap_size);
    terminal_write("Heap initialized.\n");
  } else {
    terminal_write("Heap allocation failed.\n");
  }

  timer_init(TIMER_FREQ);
  keyboard_init();

  shell_hooks_t shell_hooks;
  shell_hooks.print_mem_stats = print_mem_stats;
  shell_hooks.get_ticks = timer_get_ticks;
  shell_hooks.alloc = kmalloc;
  shell_hooks.run_memtest = run_memtest;
  shell_init(&shell_hooks);

  asm volatile("sti");
}
