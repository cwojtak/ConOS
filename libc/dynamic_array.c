#include "dynamic_array.h"

void initArray(Array* a, size_t initialSize)
{
    a->array = (uintptr_t*)mm_allocate(initialSize * sizeof(uintptr_t));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(Array* a, uintptr_t element)
{
    if(a->used == a->size)
    {
        uintptr_t* newArray = (uintptr_t*)mm_allocate(a->size * sizeof(uintptr_t) * 2);
        for(uint32_t i = 0; i < a->size; i++)
        {
            newArray[i] = a->array[i];
        }
        mm_free((uintptr_t)a->array);
        a->size *= 2;
        a->array = newArray;
    }
    a->array[a->used++] = element;
}

void freeArray(Array* a)
{
    mm_free((uintptr_t)a);
    a->array = NULL;
    a->used = a->size = 0;
}
