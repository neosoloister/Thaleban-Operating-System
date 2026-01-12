#ifndef MALLOC_H
#define MALLOC_H
#include <stddef.h>
#include <stdint.h>

void init_heap(uint32_t start_address, uint32_t size);
void *malloc(uint32_t size);
void free(void *ptr);
void mstats();

#endif