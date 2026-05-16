#ifndef MEM_PMM_H
#define MEM_PMM_H

#include <stddef.h>
#include <stdint.h>

#define PMM_FRAME_SIZE 4096u
#define PMM_MAX_MEMORY_BYTES (64u * 1024u * 1024u)

void pmm_init(uint32_t total_memory_bytes, uint32_t reserved_end_addr);
uint32_t pmm_alloc_frame(void);
uint32_t pmm_alloc_contiguous(uint32_t frame_count);
void pmm_free_frame(uint32_t physical_addr);
void pmm_reserve_range(uint32_t start_addr, uint32_t length_bytes);
void pmm_unreserve_range(uint32_t start_addr, uint32_t length_bytes);

uint32_t pmm_total_frames(void);
uint32_t pmm_used_frames(void);
uint32_t pmm_free_frames(void);

#endif
