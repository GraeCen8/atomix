#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

size_t strlen(const char *str);
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, int val, size_t count);

#include "drivers/terminal.h"
#include "mem/kmalloc.h"
#include "mem/pmm.h"

#endif
