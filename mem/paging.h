#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGING_PAGE_SIZE 4096u
#define PAGING_ENTRIES_PER_TABLE 1024u

#define PAGING_FLAG_PRESENT 0x001u
#define PAGING_FLAG_WRITABLE 0x002u
#define PAGING_FLAG_USER 0x004u

void paging_init(uint32_t kernel_end, uint32_t heap_start, uint32_t heap_size);
int paging_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
int paging_unmap(uint32_t virt_addr);
int paging_translate(uint32_t virt_addr, uint32_t *phys_out);

int paging_enabled(void);
uint32_t paging_mapped_pages(void);

#endif
