#include "keyboard.h"
#include "../idt.h"
#include "../io.h"
#include "../shell/shell.h"

static unsigned char scancode_to_char[128] = {
    0,   0,   '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

static void keyboard_irq_handler(struct registers *r) {
  (void)r;
  unsigned char scancode = inb(0x60);

  if ((scancode & 0x80u) != 0) {
    return;
  }

  char c = (char)scancode_to_char[scancode];
  if (c != 0) {
    shell_handle_key(c);
  }
}

void keyboard_init(void) { irq_install_handler(1, keyboard_irq_handler); }
