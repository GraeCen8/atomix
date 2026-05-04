#include "gdt.h"
#include "idt.h"
#include "terminal.h"

void kmain(void) {
  terminal_initialize();
  terminal_write("Atomix OS Starting...\n");

  init_gdt();
  init_idt();

  asm volatile("sti"); // enable interrupts
  while (1)
    ; // Wait for interrupts
}
