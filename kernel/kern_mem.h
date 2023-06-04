#ifndef KERN_MEM_H
#define KERN_MEM_H

#include "log.h"
#include "../libc/string.h"

struct MemoryManagerEntry {
    uintptr_t baseAddress;
    uint32_t type; //0 = AVAIL, 1 = RESRV
    uint32_t reserved;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
};

struct MemoryManager
{
	uintptr_t _memStart;
	unsigned _wordSize;
	int _blockSizeInWords;
	int (*_allocator)(int, uintptr_t);
    struct MemoryManagerEntry* _mmList;
    uintptr_t _mmStart;
    int _mmLength;
    int _mmNumEntries;
    int _mmBlockLength;
};

void create_memory_manager(unsigned wordSize, int (*allocator)(int, uintptr_t), uintptr_t mmStart, size_t objectLength);
void destroy_memory_manager();

void mm_initialize(size_t sizeInWords, uintptr_t location);
void mm_shutdown();
void mm_reserveat(size_t sizeInBytes, uintptr_t location);

void mm_logblock(int blocknum);
/*
void* mm_allocate(size_t sizeInBytes);
void mm_free(void* address);
void mm_setAllocator(std::function<int(int, void*)> allocator);
void* mm_getList();
unsigned mm_getWordSize();
void* mm_getMemoryStart();
unsigned mm_getMemoryLimit();
*/

struct MemoryMapEntry {
    uint64_t baseAddress;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

void prepare_memory_manager(struct MemoryMapEntry* mmap, size_t mmap_size);

int bestFit(int sizeInWords, uintptr_t list);
int worstFit(int sizeInWords, uintptr_t list);

#endif
