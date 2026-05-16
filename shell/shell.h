#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <stdint.h>

typedef struct shell_hooks {
  void (*print_mem_stats)(void);
  uint32_t (*get_ticks)(void);
  void *(*alloc)(size_t size);
} shell_hooks_t;

void shell_init(const shell_hooks_t *hooks);
void shell_handle_key(char c);

#endif
