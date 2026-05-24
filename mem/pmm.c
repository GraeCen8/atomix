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

static uint32_t clamped_end_frame(uint32_t start_addr, uint32_t length_bytes) {
  uint32_t max_addr = total_frames * PMM_FRAME_SIZE;
  uint32_t capped_len = length_bytes;

  if (start_addr >= max_addr) {
    return total_frames;
  }

  if (capped_len > (max_addr - start_addr)) {
    capped_len = max_addr - start_addr;
  }

  uint32_t end_exclusive = start_addr + capped_len;
  if (end_exclusive == max_addr) {
    return total_frames;
  }

  uint32_t aligned = align_up(end_exclusive, PMM_FRAME_SIZE);
  uint32_t end = aligned / PMM_FRAME_SIZE;
  if (end > total_frames) {
    end = total_frames;
  }
  return end;
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

uint32_t pmm_alloc_contiguous(uint32_t frame_count) {
  if (frame_count == 0 || used_frames >= total_frames ||
      frame_count > (total_frames - used_frames)) {
    return 0;
  }

  uint32_t run_start = 0;
  uint32_t run_length = 0;

  for (uint32_t i = 0; i < total_frames; i++) {
    if (!test_frame(i)) {
      if (run_length == 0) {
        run_start = i;
      }
      run_length++;
      if (run_length == frame_count) {
        for (uint32_t f = run_start; f < (run_start + frame_count); f++) {
          set_frame(f);
        }
        used_frames += frame_count;
        return run_start * PMM_FRAME_SIZE;
      }
    } else {
      run_length = 0;
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
  uint32_t end = clamped_end_frame(start_addr, length_bytes);

  if (start >= total_frames) {
    return;
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
  uint32_t end = clamped_end_frame(start_addr, length_bytes);

  if (start >= total_frames) {
    return;
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

uint32_t pmm_total_bytes(void) { return pmm_total_frames() * PMM_FRAME_SIZE; }

uint32_t pmm_used_bytes(void) { return pmm_used_frames() * PMM_FRAME_SIZE; }

uint32_t pmm_free_bytes(void) { return pmm_free_frames() * PMM_FRAME_SIZE; }
