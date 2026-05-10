#include "pmm.h"
#include "util.h"

#define PMM_MAX_FRAMES (PMM_MAX_MEMORY_BYTES / PMM_FRAME_SIZE)
#define PMM_BITMAP_U32S ((PMM_MAX_FRAMES + 31u) / 32u)

static uint32_t pmm_bitmap[PMM_BITMAP_U32S];
static uint32_t total_frames;
static uint32_t used_frames;

static uint32_t align_up(uint32_t value, uint32_t align) {
  return (value + align - 1u) & ~(align - 1u);
}

static void set_frame(uint32_t frame_index) {
  pmm_bitmap[frame_index / 32u] |= (1u << (frame_index % 32u));
}

static void clear_frame(uint32_t frame_index) {
  pmm_bitmap[frame_index / 32u] &= ~(1u << (frame_index % 32u));
}

static int test_frame(uint32_t frame_index) {
  return (pmm_bitmap[frame_index / 32u] & (1u << (frame_index % 32u))) != 0;
}

void pmm_init(uint32_t total_memory_bytes, uint32_t reserved_end_addr) {
  uint32_t capped_bytes = total_memory_bytes;
  if (capped_bytes > PMM_MAX_MEMORY_BYTES) {
    capped_bytes = PMM_MAX_MEMORY_BYTES;
  }

  total_frames = capped_bytes / PMM_FRAME_SIZE;
  used_frames = total_frames;

  memset(pmm_bitmap, 0xFF, sizeof(pmm_bitmap));

  if (total_frames == 0) {
    return;
  }

  uint32_t free_start = align_up(reserved_end_addr, PMM_FRAME_SIZE);
  if (free_start >= capped_bytes) {
    return;
  }

  pmm_unreserve_range(free_start, capped_bytes - free_start);
}

uint32_t pmm_alloc_frame(void) {
  if (used_frames >= total_frames) {
    return 0;
  }

  for (uint32_t i = 0; i < total_frames; i++) {
    if (!test_frame(i)) {
      set_frame(i);
      used_frames++;
      return i * PMM_FRAME_SIZE;
    }
  }

  return 0;
}

void pmm_free_frame(uint32_t physical_addr) {
  if ((physical_addr % PMM_FRAME_SIZE) != 0) {
    return;
  }

  uint32_t frame_index = physical_addr / PMM_FRAME_SIZE;
  if (frame_index >= total_frames) {
    return;
  }

  if (test_frame(frame_index)) {
    clear_frame(frame_index);
    if (used_frames > 0) {
      used_frames--;
    }
  }
}

void pmm_reserve_range(uint32_t start_addr, uint32_t length_bytes) {
  if (length_bytes == 0 || total_frames == 0) {
    return;
  }

  uint32_t start = start_addr / PMM_FRAME_SIZE;
  uint32_t end_addr = align_up(start_addr + length_bytes, PMM_FRAME_SIZE);
  uint32_t end = end_addr / PMM_FRAME_SIZE;

  if (start >= total_frames) {
    return;
  }

  if (end > total_frames) {
    end = total_frames;
  }

  for (uint32_t i = start; i < end; i++) {
    if (!test_frame(i)) {
      set_frame(i);
      used_frames++;
    }
  }
}

void pmm_unreserve_range(uint32_t start_addr, uint32_t length_bytes) {
  if (length_bytes == 0 || total_frames == 0) {
    return;
  }

  uint32_t start = start_addr / PMM_FRAME_SIZE;
  uint32_t end_addr = align_up(start_addr + length_bytes, PMM_FRAME_SIZE);
  uint32_t end = end_addr / PMM_FRAME_SIZE;

  if (start >= total_frames) {
    return;
  }

  if (end > total_frames) {
    end = total_frames;
  }

  for (uint32_t i = start; i < end; i++) {
    if (test_frame(i)) {
      clear_frame(i);
      if (used_frames > 0) {
        used_frames--;
      }
    }
  }
}

uint32_t pmm_total_frames(void) { return total_frames; }

uint32_t pmm_used_frames(void) { return used_frames; }

uint32_t pmm_free_frames(void) { return total_frames - used_frames; }
