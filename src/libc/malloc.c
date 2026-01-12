#include "malloc.h"
#include "../kernel/kprintf.h"
#include <stdint.h>

typedef struct malloc_header {
    uint32_t size;
    uint8_t is_free;
    uint32_t magic;    // Set to 0xCAFEBABE
    struct malloc_header *next;
} malloc_header_t;

static malloc_header_t *heap_head = NULL;

void init_heap(uint32_t start_address, uint32_t size) {
    heap_head = (malloc_header_t *)start_address;
    heap_head->size = size - sizeof(malloc_header_t);
    heap_head->is_free = 1;
    heap_head->magic = 0xCAFEBABE;
    heap_head->next = NULL;
}

void *malloc(uint32_t size) {
    if (size == 0) return NULL;

    // Align size to 8 bytes
    if (size % 8 != 0) size = (size + 7) & ~7;

    malloc_header_t *current = heap_head;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split block if enough space
            if (current->size > size + sizeof(malloc_header_t)) {
                malloc_header_t *new_block = (malloc_header_t *)((uint8_t *)current + sizeof(malloc_header_t) + size);
                new_block->size = current->size - size - sizeof(malloc_header_t);
                new_block->is_free = 1;
                new_block->magic = 0xCAFEBABE;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            current->is_free = 0;
            return (void *)((uint8_t *)current + sizeof(malloc_header_t));
        }
        current = current->next;
    }
    return NULL; // Out of memory
}

void free(void *ptr) {
    if (!ptr) return;

    malloc_header_t *header = (malloc_header_t *)((uint8_t *)ptr - sizeof(malloc_header_t));
    if (header->magic != 0xCAFEBABE) return; // Corruption or invalid pointer

    header->is_free = 1;

    // Coalesce with next block if free
    if (header->next && header->next->is_free) {
        header->size += sizeof(malloc_header_t) + header->next->size;
        header->next = header->next->next;
    }
}

void mstats() {
    kprintf("Memory Statistics:\n");
    malloc_header_t *current = heap_head;
    uint32_t total_mem = 0;
    uint32_t free_mem = 0;
    uint32_t used_mem = 0;
    uint32_t overhead = 0;

    while (current) {
        overhead += sizeof(malloc_header_t);
        total_mem += current->size;
        if (current->is_free) {
            free_mem += current->size;
            kprintf(" [FREE] Addr: %x Size: %d\n", (uint32_t)current, current->size);
        } else {
            used_mem += current->size;
            kprintf(" [USED] Addr: %x Size: %d\n", (uint32_t)current, current->size);
        }
        current = current->next;
    }
    kprintf("Total Usable: %d bytes\n", total_mem);
    kprintf("Used: %d bytes\n", used_mem);
    kprintf("Free: %d bytes\n", free_mem);
    kprintf("Overhead: %d bytes\n", overhead);
}