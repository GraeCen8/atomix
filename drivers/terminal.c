#include "terminal.h"
#include "../util.h"
#include <stddef.h>
#include <stdint.h>

volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
volatile uint16_t *terminal_buffer;

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
  return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_clear(void) {
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }

  terminal_row = 0;
  terminal_column = 0;
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(7, 0);
  terminal_buffer = vga_buffer;
  terminal_clear();
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
  } else if (c == '\r') {
    terminal_column = 0;
  } else if (c == '\b') {
    if (terminal_column > 0) {
      terminal_column--;
      const size_t index = terminal_row * VGA_WIDTH + terminal_column;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  } else {
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry((unsigned char)c, terminal_color);
    terminal_column++;
  }

  if (terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    terminal_row++;
  }

  if (terminal_row == VGA_HEIGHT) {
    size_t row;
    for (row = 1; row < VGA_HEIGHT; row++) {
      memcpy((void *)((row - 1) * VGA_WIDTH * 2 + (uint32_t)vga_buffer),
             (const void *)(row * VGA_WIDTH * 2 + (uint32_t)vga_buffer),
             VGA_WIDTH * 2);
    }

    size_t last_row = VGA_HEIGHT - 1;
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = last_row * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
    terminal_row--;
  }
}

void terminal_write(const char *data) {
  size_t datalen = strlen(data);
  for (size_t i = 0; i < datalen; i++) {
    terminal_putchar(data[i]);
  }
}
