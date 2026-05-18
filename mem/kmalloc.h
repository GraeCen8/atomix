#ifndef MEM_KMALLOC_H
#define MEM_KMALLOC_H

#include <stddef.h>
#include <stdint.h>

void kmalloc_init(uint32_t heap_start, uint32_t heap_size_bytes);
void *kmalloc(size_t size);
void kfree(void *ptr);
void kmalloc_defrag(void);

#endif
