#include "drivers/terminal.h"
#include "gdt.h"
#include "idt.h"
#include "mem/kmalloc.h"
#include "mem/pmm.h"
#include "shell/shell.h"
#include "timer.h"
#include <stdint.h>

void boot(void);
extern uint8_t __kernel_end;
static uint32_t heap_base;
static uint32_t heap_size;

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

  if (heap_size > 0) {
    terminal_write("Heap base/size bytes: ");
    terminal_write_u32(heap_base);
    terminal_write("/");
    terminal_write_u32(heap_size);
    terminal_write("\n");
  }
}

void kmain(void) {
  boot();

  while (1) {
    asm volatile("hlt");
  }
}

void boot(void) {
  const uint32_t total_memory_bytes = PMM_MAX_MEMORY_BYTES;
  const uint32_t heap_size_bytes = 1024u * 1024u;

  terminal_initialize();
  terminal_write("Atomix OS Starting...\n");

  init_gdt();
  init_idt();

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

  shell_hooks_t shell_hooks;
  shell_hooks.print_mem_stats = print_mem_stats;
  shell_hooks.get_ticks = timer_get_ticks;
  shell_hooks.alloc = kmalloc;
  shell_init(&shell_hooks);

  asm volatile("sti");
}
