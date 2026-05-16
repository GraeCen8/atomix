#include "drivers/terminal.h"
#include "gdt.h"
#include "idt.h"
#include "mem/kmalloc.h"
#include "mem/pmm.h"
#include "timer.h"
#include <stdint.h>

void boot(void);
extern uint8_t __kernel_end;

void kmain(void) {
  boot();

  while (1) {
    sleep(TIMER_FREQ / 10); // 10 times per second
    terminal_write("tick\n");
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
    kmalloc_init(heap_start, pages * PMM_FRAME_SIZE);
  }

  timer_init(TIMER_FREQ);

  asm volatile("sti"); // enable interrupts
}
