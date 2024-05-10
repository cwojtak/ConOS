#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

#include "../kernel/kern_mem.h"

typedef struct {
    uintptr_t* array;
    size_t used;
    size_t size;
} Array;

void initArray(Array* a, size_t initialSize);
void insertArray(Array* a, uintptr_t element);
void freeArray(Array* a);

#endif
