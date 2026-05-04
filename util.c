int strlen(const char *str) {
  int len = 0;
  while (str[len] != '\0')
    len++;
  return len;
}

void memcpy(void *dest, const void *src, int count) {
  char *cdest = (char *)dest;
  char *csrc = (char *)src;
  int i;
  for (i = 0; i < count; i++) {
    cdest[i] = csrc[i];
  }
}

void memset(void *dest, char val, int count) {
  char *cdest = (char *)dest;
  int i;
  for (i = 0; i < count; i++) {
    cdest[i] = val;
  }
}
