#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry_struct {
  uint16_t base_low;
  uint16_t sel;    // Kernel segment selector
  uint8_t always0; // Unused, set to 0
  uint8_t flags;   // Present, Ring 0, etc.
  uint16_t base_high;
} __attribute__((packed));

struct idt_ptr_struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

// This struct MUST match the order registers are pushed in isr.asm
struct registers {
  uint32_t ds, es, fs, gs;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*irq_handler_t)(struct registers *r);

void init_idt();
void isr_handler(struct registers *r); // this is called on interrupts
void irq_handler(struct registers *r); // handle hardware (eg: Keyboard)
void irq_install_handler(int irq, irq_handler_t handler);
void irq_uninstall_handler(int irq);

#endif
