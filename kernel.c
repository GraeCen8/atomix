#include "drivers/terminal.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include <stdint.h>

void boot(void);

void kmain(void) {
  boot();

  while (1) {
    sleep(TIMER_FREQ / 10);
    terminal_write("tick\n");
  }
}

void boot(void) {
  terminal_initialize();
  terminal_write("Atomix OS Starting...\n");

  init_gdt();
  init_idt();
  timer_init(TIMER_FREQ);

  asm volatile("sti"); // enable interrupts
}
