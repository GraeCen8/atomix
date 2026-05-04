#include "terminal.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// VGA memory location
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;

// screen dimensions
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

// terminal state
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

// Color helper: Combine foreground and background
// 0x0F is White on Black
static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
  return fg | (bg << 4);
}

// Character helper: Combine the char and color
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(7, 0); // light grey on black
  terminal_buffer = vga_buffer;

  // clear the screen
  for (site_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_putchar(char c) {
  // Handle the newline if there is one
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
  } else {
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry((unsigned char)c, terminal_color);
    terminal_row++;
  }

  // we need to handle scrolling if we hit the bottom
  if (terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    terminal_row++;
  }

  if (terminal_row == VGA_HEIGHT) {
    // Move all the text up one line
    size_t row;
    for (row = 1; row < VGA_HEIGHT; row++) {
      memcpy((void *)((row - 1) * VGA_WIDTH * 2 + (uint32_t)vga_buffer),
             (void *)(row * VGA_WIDTH * 2 + (uint32_t)vga_buffer),
             VGA_WIDTH * 2);
    }
    // clear the last line
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
  for (size_t i = 0; i < datalen; i++)
    terminal_putchar(data[i]);
}
