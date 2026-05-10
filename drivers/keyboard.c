#include "keyboard.h"
#include "io.h"
#include "terminal.h"

unsigned char scancode_to_char[128] = {
    0,   0,   '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ' // Spacebar
};

void keyboard_handler() {
  unsigned char scancode = inb(0x60);
  // Check if the key was pressed (top bit is 0) vs released (top bit is 1)
  if (scancode & 0x80) {
    // Key released (do nothing for now)
  } else {
    char c = scancode_to_char[scancode];
    if (c != 0) {
      terminal_putchar(c);
    }
  }
}
