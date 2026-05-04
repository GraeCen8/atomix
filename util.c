#include "util.h"

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len] != '\0')
    len++;
  return len;
}

void *memcpy(void *dest, const void *src, size_t count) {
  char *cdest = (char *)dest;
  const char *csrc = (const char *)src;
  size_t i;
  for (i = 0; i < count; i++) {
    cdest[i] = csrc[i];
  }
  return dest;
}

void *memset(void *dest, int val, size_t count) {
  char *cdest = (char *)dest;
  size_t i;
  for (i = 0; i < count; i++) {
    cdest[i] = (char)val;
  }
  return dest;
}
