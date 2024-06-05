#include "dynamic_array.h"

void initArray(Array* a, size_t initialSize)
{
    a->array = mm_allocate(initialSize * sizeof(uintptr_t));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(Array* a, uintptr_t element)
{
    if(a->used == a->size)
    {
        uintptr_t newArray = mm_allocate(a->size * sizeof(uintptr_t) * 2);
        for(uint32_t i = 0; i < a->size; i++)
        {
            (*(uintptr_t*)newArray) = (*(uintptr_t*)a->array);
            newArray += sizeof(uintptr_t);
            a->array += sizeof(uintptr_t);
        }
        mm_free(a->array);
        a->size *= 2;
        a->array = newArray;
    }
    uintptr_t elem_addr = a->array + (a->used++) * sizeof(uintptr_t);
    *(uintptr_t*)(elem_addr)= element;
}

void freeArray(Array* a)
{
    mm_free(a->array);
    a->array = (uintptr_t)NULL;
    a->used = a->size = 0;
}

void strsplit_indices(char str[], char splitOn, Array* output)
{
    initArray(output, 8);
    uint32_t substringEnd = 0;
    for(uint32_t i = 0; i < strlen(str); i++)
    {
        if(str[i] == splitOn)
        {
            insertArray(output, substringEnd);
        }
        substringEnd++;
    }
}