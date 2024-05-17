#ifndef KERN_MEM_H
#define KERN_MEM_H

#include "log.h"
#include "../libc/string.h"

struct MemoryManagerEntry {
    uint32_t type; //0 = AVAIL, 1 = RESRV
    struct MemoryManagerEntry* lastContig;
    struct MemoryManagerEntry* firstContig;
    uint32_t reserved;
};

struct MemoryManager
{
	uintptr_t _memStart;
	unsigned _wordSize;
	int _blockSizeInWords;
	struct MemoryManagerEntry* (*_allocator)(size_t, struct MemoryManagerEntry*);
    struct MemoryManagerEntry* _mmList;
    struct MemoryManagerEntry* _mmListEnd;
    uintptr_t _mmStart;
    int _mmLength;
    int _mmNumEntries;
    int _mmBlockLength;
};

void create_memory_manager(unsigned wordSize, struct MemoryManagerEntry* (*allocator)(size_t, struct MemoryManagerEntry*), uintptr_t mmStart, size_t objectLength);
void destroy_memory_manager();

void mm_initialize(size_t sizeInWords, uintptr_t location);
void mm_shutdown();
void mm_reserveat(size_t sizeInBytes, uintptr_t location);
uintptr_t mm_allocate(size_t sizeInBytes);
int mm_free(uintptr_t address);
void mm_setAllocator(struct MemoryManagerEntry* (*allocator)(size_t, struct MemoryManagerEntry*));
struct MemoryManagerEntry* mm_getList();
unsigned mm_getWordSize();
uintptr_t mm_getMemoryStart();
unsigned mm_getMemoryLimit();

void mm_logblock(int blocknum);

struct MemoryMapEntry {
    uint64_t baseAddress;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

void prepare_memory_manager(struct MemoryMapEntry* mmap, size_t mmap_size);

struct MemoryManagerEntry* bestFit(size_t sizeInBytes, struct MemoryManagerEntry* list);

#endif
