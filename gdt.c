#include "gdt.h"
#include "util.h"
#include <stdint.h>

struct gdt_entry_struct gdt_entries[5];
struct gdt_ptr_struct gdt_ptr;

extern void gdt_flush(uint32_t);
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran);

void init_gdt() {
  // setup the gdt pointer
  gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 5) - 1;
  gdt_ptr.base = (uint32_t)&gdt_entries;

  // set up the gdt gdt_entries
  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFF, 0xCF); // User code segment

  gdt_flush((uint32_t)&gdt_ptr);
}

void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran) {
  gdt_entries[num].base_low = (base & 0xFFFF);
  gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  gdt_entries[num].base_high = (base >> 24) & 0xFF;

  gdt_entries[num].limit_low = (limit & 0xFFFF);
  gdt_entries[num].granularity = (limit >> 16) & 0x0F;
  gdt_entries[num].granularity |= gran & 0xF0;
  gdt_entries[num].access = access;
}
