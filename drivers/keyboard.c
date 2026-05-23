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

static unsigned char scancode_to_shift_char[128] = {
    0,   0,   '!',  '@',  '#',  '$',  '%', '^',  '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W',  'E', 'R',  'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,    'A', 'S',  'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~',  0,   '|',  'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?',  0,   '*',  0,   ' '};

static int g_shift_down;
static int g_ctrl_down;
static int g_caps_lock;

static int is_alpha(char c) { return c >= 'a' && c <= 'z'; }

static char to_upper(char c) {
  if (c >= 'a' && c <= 'z') {
    return (char)(c - ('a' - 'A'));
  }
  return c;
}

static void keyboard_irq_handler(struct registers *r) {
  (void)r;
  unsigned char scancode = inb(0x60);
  unsigned char code = scancode & 0x7Fu;

  if (code == 0x2A || code == 0x36) {
    g_shift_down = (scancode & 0x80u) == 0;
    return;
  }

  if (code == 0x1D) {
    g_ctrl_down = (scancode & 0x80u) == 0;
    return;
  }

  if ((scancode & 0x80u) != 0) {
    return;
  }

  if (scancode == 0x3A) {
    g_caps_lock = !g_caps_lock;
    return;
  }

  char c = (char)(g_shift_down ? scancode_to_shift_char[scancode]
                               : scancode_to_char[scancode]);

  if (c == 0) {
    return;
  }

  if (is_alpha(c) && (g_shift_down ^ g_caps_lock)) {
    c = to_upper(c);
  }

  if (g_ctrl_down) {
    if (c >= 'A' && c <= 'Z') {
      c = (char)(c - 'A' + 1);
    } else if (c >= 'a' && c <= 'z') {
      c = (char)(c - 'a' + 1);
    }
  }

  if (c != 0) {
    shell_handle_key(c);
  }
}

void keyboard_init(void) { irq_install_handler(1, keyboard_irq_handler); }
