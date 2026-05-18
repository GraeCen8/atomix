#include "timer.h"
#include "idt.h"
#include "io.h"
#include <stdint.h>

// global counter
volatile uint32_t timer_ticks = 0;

static void timer_irq_handler(struct registers *r) {
  (void)r;
  timer_ticks++;
}

void timer_init(uint32_t frequency) {
  // the pit runs at 1193180 Hz by default.
  // to get the desired fequency, we divide the input by the frequency.
  uint32_t divisor = 1193180 / frequency;

  // Send Command Byte to PIT Command Port (0x43)
  // Bits: 0b00110100
  // 00: Channel 0
  // 11: Read/Write LSB then MSB
  // 011: Square Wave Generator
  // 0: 16-bit binary mode
  uint8_t command = 0b00110100;
  outb(0x43, command);

  // Send the divisor (low byte first then high byte) to channel 0 data port
  // (0x40)
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

  outb(0x40, low);
  outb(0x40, high);
  irq_install_handler(0, timer_irq_handler);
}

uint32_t timer_get_ticks() { return timer_ticks; }

void sleep(int ticks) {
  if (ticks <= 0) {
    return;
  }

  while (ticks-- > 0) {
    uint32_t current_tick = timer_get_ticks();
    while (timer_get_ticks() == current_tick) {
      asm volatile("hlt");
    }
  }
}
