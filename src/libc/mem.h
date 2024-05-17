#ifndef MEM_H
#define MEM_H

#include <stddef.h>

#include "../cpu/type.h"

void memory_copy(uint8_t *source, uint8_t *dest, size_t nbytes);
void memory_set(uint8_t *dest, uint8_t val, uint32_t len);

uint32_t kmalloc(size_t size, int align, uint32_t *phys_addr);

#endif
