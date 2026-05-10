#include "idt.h"
#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "io.h"
#include "timer.h"
#include "util.h"

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
                         uint8_t flags);
static void pic_remap(void);

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct idt_ptr;

extern void idt_flush(uint32_t idt_ptr);

extern void isr0(void);
extern void isr1(void);
extern void irq0(void);
extern void irq1(void);

void init_idt() {
  idt_ptr.limit = sizeof(struct idt_entry_struct) * 256 - 1;
  idt_ptr.base = (uint32_t)&idt_entries;

  memset(&idt_entries, 0, sizeof(struct idt_entry_struct) * 256);

  pic_remap();

  idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
  idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
  idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
  idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);

  idt_flush((uint32_t)&idt_ptr);
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
                         uint8_t flags) {
  idt_entries[num].base_low = (base & 0xFFFF);
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  idt_entries[num].sel = sel;
  idt_entries[num].always0 = 0;
  idt_entries[num].flags = flags;
}

static void pic_remap(void) {
  unsigned char a1 = inb(0x21);
  unsigned char a2 = inb(0xA1);

  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // Unmask only IRQ0 (timer) and IRQ1 (keyboard) on master.
  outb(0x21, (a1 & 0xFC));
  outb(0xA1, a2);
}

void isr_handler(struct registers *r) {
  (void)r;
  terminal_write("CPU exception\n");
  while (1)
    ;
}

void irq_handler(struct registers *r) {
  if (r->int_no == 33) {
    keyboard_handler();
  }

  if (r->int_no == 32) {
    timer_handler();
  }

  if (r->int_no >= 40) {
    outb(0xA0, 0x20);
  }
  outb(0x20, 0x20);
}
