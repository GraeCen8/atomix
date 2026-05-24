#include "kmalloc.h"

#define KMALLOC_ALIGN 8u

typedef struct block_header {
  size_t size;
  uint32_t magic;
  int free;
  struct block_header *next;
} block_header_t;

static block_header_t *free_list_head;
static uintptr_t heap_region_start;
static uintptr_t heap_region_end;
static const uint32_t KMALLOC_MAGIC = 0xC0DEFACEu;

static size_t align_up(size_t value) {
  return (value + (KMALLOC_ALIGN - 1u)) & ~(KMALLOC_ALIGN - 1u);
}

void kmalloc_init(uint32_t heap_start, uint32_t heap_size_bytes) {
  if (heap_size_bytes <= sizeof(block_header_t)) {
    free_list_head = 0;
    heap_region_start = 0;
    heap_region_end = 0;
    return;
  }

  heap_region_start = (uintptr_t)heap_start;
  heap_region_end = heap_region_start + heap_size_bytes;
  free_list_head = (block_header_t *)(uintptr_t)heap_start;
  free_list_head->size = heap_size_bytes - sizeof(block_header_t);
  free_list_head->magic = KMALLOC_MAGIC;
  free_list_head->free = 1;
  free_list_head->next = 0;
}

static void split_block(block_header_t *block, size_t size) {
  if (block->size <= size + sizeof(block_header_t) + KMALLOC_ALIGN) {
    return;
  }

  uintptr_t base = (uintptr_t)block;
  uintptr_t split_at = base + sizeof(block_header_t) + size;
  block_header_t *new_block = (block_header_t *)split_at;

  new_block->size = block->size - size - sizeof(block_header_t);
  new_block->magic = KMALLOC_MAGIC;
  new_block->free = 1;
  new_block->next = block->next;

  block->size = size;
  block->next = new_block;
}

void *kmalloc(size_t size) {
  if (size == 0 || free_list_head == 0) {
    return 0;
  }

  size = align_up(size);

  block_header_t *curr = free_list_head;
  while (curr != 0) {
    if (curr->free && curr->size >= size) {
      split_block(curr, size);
      curr->free = 0;
      return (void *)((uintptr_t)curr + sizeof(block_header_t));
    }
    curr = curr->next;
  }

  return 0;
}

static void coalesce_free_blocks(void) {
  block_header_t *curr = free_list_head;
  while (curr != 0 && curr->next != 0) {
    uintptr_t curr_end =
        (uintptr_t)curr + sizeof(block_header_t) + curr->size;
    if (curr->free && curr->next->free && curr_end == (uintptr_t)curr->next) {
      curr->size += sizeof(block_header_t) + curr->next->size;
      curr->next = curr->next->next;
    } else {
      curr = curr->next;
    }
  }
}

void kfree(void *ptr) {
  if (ptr == 0) {
    return;
  }

  uintptr_t ptr_addr = (uintptr_t)ptr;
  if (ptr_addr < (heap_region_start + sizeof(block_header_t)) ||
      ptr_addr >= heap_region_end) {
    return;
  }

  block_header_t *block = 0;
  block_header_t *curr = free_list_head;
  while (curr != 0) {
    uintptr_t payload = (uintptr_t)curr + sizeof(block_header_t);
    if (payload == ptr_addr) {
      block = curr;
      break;
    }
    curr = curr->next;
  }

  if (block == 0) {
    return;
  }

  uintptr_t block_addr = (uintptr_t)block;
  if (block->magic != KMALLOC_MAGIC || block->free ||
      (block_addr + sizeof(block_header_t) + block->size) > heap_region_end) {
    return;
  }

  block->free = 1;
  coalesce_free_blocks();
}

void kmalloc_defrag(void) { coalesce_free_blocks(); }
