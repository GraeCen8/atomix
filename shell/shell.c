#include "shell.h"
#include "../drivers/terminal.h"
#include "../mem/paging.h"
#include "../util.h"

#define SHELL_INPUT_MAX 128
#define SHELL_ARGV_MAX 8

static shell_hooks_t g_hooks;
static char g_input[SHELL_INPUT_MAX];
static size_t g_input_len;

static void terminal_write_u32(uint32_t value) {
  if (value == 0) {
    terminal_putchar('0');
    return;
  }

  uint32_t div = 1000000000u;
  while (div > 0 && (value / div) == 0) {
    div /= 10u;
  }

  while (div > 0) {
    uint32_t digit = value / div;
    terminal_putchar((char)('0' + digit));
    value %= div;
    div /= 10u;
  }
}

static void terminal_write_hex(uint32_t value) {
  static const char *hex = "0123456789ABCDEF";
  terminal_write("0x");
  for (int i = 7; i >= 0; i--) {
    uint32_t nibble = (value >> (i * 4)) & 0xFu;
    terminal_putchar(hex[nibble]);
  }
}

static int streq(const char *a, const char *b) {
  size_t i = 0;
  while (a[i] != '\0' && b[i] != '\0') {
    if (a[i] != b[i]) {
      return 0;
    }
    i++;
  }
  return a[i] == b[i];
}

static int parse_u32(const char *s, uint32_t *out) {
  if (s == 0 || s[0] == '\0') {
    return 0;
  }

  uint32_t value = 0;
  size_t i = 0;
  while (s[i] != '\0') {
    if (s[i] < '0' || s[i] > '9') {
      return 0;
    }
    value = (value * 10u) + (uint32_t)(s[i] - '0');
    i++;
  }

  *out = value;
  return 1;
}

static int parse_u32_auto(const char *s, uint32_t *out) {
  if (s == 0 || s[0] == '\0') {
    return 0;
  }

  uint32_t value = 0;
  uint32_t base = 10;
  size_t i = 0;

  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    base = 16;
    i = 2;
    if (s[i] == '\0') {
      return 0;
    }
  }

  while (s[i] != '\0') {
    uint32_t digit;

    if (s[i] >= '0' && s[i] <= '9') {
      digit = (uint32_t)(s[i] - '0');
    } else if (base == 16 && s[i] >= 'a' && s[i] <= 'f') {
      digit = (uint32_t)(s[i] - 'a' + 10);
    } else if (base == 16 && s[i] >= 'A' && s[i] <= 'F') {
      digit = (uint32_t)(s[i] - 'A' + 10);
    } else {
      return 0;
    }

    if (digit >= base) {
      return 0;
    }

    value = (value * base) + digit;
    i++;
  }

  *out = value;
  return 1;
}

static int split_args(char *line, char **argv, int argv_max) {
  int argc = 0;
  size_t i = 0;

  while (line[i] != '\0') {
    while (line[i] == ' ' || line[i] == '\t') {
      i++;
    }

    if (line[i] == '\0') {
      break;
    }

    if (argc >= argv_max) {
      break;
    }

    argv[argc++] = &line[i];

    while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t') {
      i++;
    }

    if (line[i] == '\0') {
      break;
    }

    line[i++] = '\0';
  }

  return argc;
}

static void shell_prompt(void) { terminal_write("atomix> "); }

static void execute_command(char *line) {
  char *argv[SHELL_ARGV_MAX];
  int argc = split_args(line, argv, SHELL_ARGV_MAX);
  if (argc == 0) {
    return;
  }

  if (streq(argv[0], "help")) {
    terminal_write(
        "Built-ins: help, mem, memtest, ticks, clear, halt, alloc <n>, vm, "
        "translate <addr>\n");
    return;
  }

  if (streq(argv[0], "vm")) {
    terminal_write("paging: ");
    terminal_write(paging_enabled() ? "enabled" : "disabled");
    terminal_write(" mapped_pages=");
    terminal_write_u32(paging_mapped_pages());
    terminal_write("\n");
    return;
  }

  if (streq(argv[0], "mem")) {
    if (g_hooks.print_mem_stats != 0) {
      g_hooks.print_mem_stats();
    }
    return;
  }

  if (streq(argv[0], "ticks")) {
    terminal_write("ticks: ");
    if (g_hooks.get_ticks != 0) {
      terminal_write_u32(g_hooks.get_ticks());
    } else {
      terminal_write("0");
    }
    terminal_write("\n");
    return;
  }

  if (streq(argv[0], "memtest")) {
    if (g_hooks.run_memtest == 0) {
      terminal_write("memtest unavailable\n");
      return;
    }

    if (g_hooks.run_memtest()) {
      terminal_write("memtest: PASS\n");
    } else {
      terminal_write("memtest: FAIL\n");
    }
    return;
  }

  if (streq(argv[0], "clear")) {
    terminal_initialize();
    return;
  }

  if (streq(argv[0], "halt")) {
    terminal_write("CPU halted.\n");
    asm volatile("cli");
    while (1) {
      asm volatile("hlt");
    }
  }

  if (streq(argv[0], "alloc")) {
    if (argc < 2) {
      terminal_write("usage: alloc <bytes>\n");
      return;
    }

    uint32_t bytes = 0;
    if (!parse_u32(argv[1], &bytes) || bytes == 0) {
      terminal_write("alloc: expected a positive decimal integer\n");
      return;
    }

    if (g_hooks.alloc == 0) {
      terminal_write("alloc: allocator unavailable\n");
      return;
    }

    void *ptr = g_hooks.alloc((size_t)bytes);
    if (ptr == 0) {
      terminal_write("alloc failed\n");
      return;
    }

    terminal_write("allocated ");
    terminal_write_u32(bytes);
    terminal_write(" bytes at ");
    terminal_write_hex((uint32_t)(uintptr_t)ptr);
    terminal_write("\n");
    return;
  }

  if (streq(argv[0], "translate")) {
    if (argc < 2) {
      terminal_write("usage: translate <addr>\n");
      return;
    }

    uint32_t virt = 0;
    if (!parse_u32_auto(argv[1], &virt)) {
      terminal_write("translate: expected decimal or 0xHEX address\n");
      return;
    }

    uint32_t phys = 0;
    if (!paging_translate(virt, &phys)) {
      terminal_write("unmapped\n");
      return;
    }

    terminal_write("virt ");
    terminal_write_hex(virt);
    terminal_write(" -> phys ");
    terminal_write_hex(phys);
    terminal_write("\n");
    return;
  }

  terminal_write("unknown command: ");
  terminal_write(argv[0]);
  terminal_write("\n");
}

void shell_init(const shell_hooks_t *hooks) {
  memset(&g_hooks, 0, sizeof(g_hooks));
  if (hooks != 0) {
    g_hooks.print_mem_stats = hooks->print_mem_stats;
    g_hooks.get_ticks = hooks->get_ticks;
    g_hooks.alloc = hooks->alloc;
    g_hooks.run_memtest = hooks->run_memtest;
  }

  g_input_len = 0;
  memset(g_input, 0, sizeof(g_input));

  terminal_write("Tiny shell v1 ready. Type 'help'.\n");
  shell_prompt();
}

void shell_handle_key(char c) {
  if (c == 3) {
    terminal_write("^C\n");
    g_input_len = 0;
    g_input[0] = '\0';
    shell_prompt();
    return;
  }

  if (c == 12) {
    terminal_initialize();
    shell_prompt();
    if (g_input_len > 0) {
      terminal_write(g_input);
    }
    return;
  }

  if (c == '\r' || c == '\n') {
    terminal_putchar('\n');
    g_input[g_input_len] = '\0';
    execute_command(g_input);

    g_input_len = 0;
    g_input[0] = '\0';
    shell_prompt();
    return;
  }

  if (c == '\b') {
    if (g_input_len > 0) {
      g_input_len--;
      g_input[g_input_len] = '\0';
      terminal_putchar('\b');
    }
    return;
  }

  if (c < 32 || c > 126) {
    return;
  }

  if (g_input_len >= (SHELL_INPUT_MAX - 1)) {
    return;
  }

  g_input[g_input_len++] = c;
  g_input[g_input_len] = '\0';
  terminal_putchar(c);
}
