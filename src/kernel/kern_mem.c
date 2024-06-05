#include "kern_mem.h"

struct MemoryManager* theMemoryManager = (struct MemoryManager*)NULL;

void create_memory_manager(unsigned wordSize, struct MemoryManagerEntry* (*allocator)(size_t, struct MemoryManagerEntry*), uintptr_t mmStart, size_t objectLength)
{
    theMemoryManager = (struct MemoryManager*)mmStart;
	theMemoryManager->_mmStart = mmStart;
	theMemoryManager->_allocator = allocator;
	theMemoryManager->_memStart = (uintptr_t)NULL;
	theMemoryManager->_mmList = NULL;
	theMemoryManager->_mmListEnd = NULL;
	theMemoryManager->_mmLength = objectLength;
	theMemoryManager->_wordSize = wordSize;
}

void destroy_memory_manager()
{
    mm_shutdown();
}

void mm_initialize(size_t sizeInWords, uintptr_t location)
{
    if (theMemoryManager->_memStart != (uintptr_t)NULL)
    {
        log(4, "Attempted to re-initialize the memory manager without first shutting it down. Ignoring initialize request.");
        return;
    }

    theMemoryManager->_memStart = location;
    theMemoryManager->_blockSizeInWords = sizeInWords;
    theMemoryManager->_mmList = (struct MemoryManagerEntry*)(theMemoryManager->_mmStart + sizeof(*theMemoryManager));

    theMemoryManager->_mmNumEntries = (theMemoryManager->_mmLength - sizeof(*theMemoryManager)) / sizeof(theMemoryManager->_mmList[0]);

	theMemoryManager->_mmBlockLength = sizeInWords * theMemoryManager->_wordSize / theMemoryManager->_mmNumEntries;
	if(theMemoryManager->_mmBlockLength == 0) theMemoryManager->_mmBlockLength = 1;

	struct MemoryManagerEntry* current_ent = theMemoryManager->_mmList;
	for(int i = 0; i < theMemoryManager->_mmNumEntries; i++)
	{
		current_ent->type = 0;
		current_ent->firstContig = theMemoryManager->_mmList;
		current_ent->reserved = 0;

		current_ent++;
	}

	current_ent--;
	theMemoryManager->_mmListEnd = current_ent;

	current_ent = theMemoryManager->_mmList;
	for(int i = 0; i < theMemoryManager->_mmNumEntries; i++)
	{
		current_ent->lastContig = theMemoryManager->_mmListEnd;
		current_ent++;
	}
}

void mm_shutdown()
{
    theMemoryManager->_memStart = (uintptr_t)NULL;
    theMemoryManager->_mmList = NULL;
    theMemoryManager->_blockSizeInWords = 0;
	theMemoryManager->_mmStart = (uintptr_t)NULL;
    theMemoryManager->_mmLength = 0;
    theMemoryManager->_mmNumEntries = 0;
    theMemoryManager->_mmBlockLength = 0;
	theMemoryManager->_mmListEnd = NULL;
}

void mm_reserveat(size_t sizeInBytes, uintptr_t location)
{
    //Ensure the sizeInBytes is valid
	if (sizeInBytes < 0) return;

	//Calculate starting entry
	int sizeInBlocks = sizeInBytes / theMemoryManager->_mmBlockLength;
	struct MemoryManagerEntry* start_ent = theMemoryManager->_mmList;
	start_ent += (location - theMemoryManager->_memStart) / theMemoryManager->_mmBlockLength;

    //Sometimes the memory manager doesn't quite cover all of memory, so return if the location is outside of memory
    if(start_ent > theMemoryManager->_mmListEnd || start_ent + (sizeInBlocks / theMemoryManager->_mmBlockLength) > theMemoryManager->_mmListEnd
        || start_ent < theMemoryManager->_mmList
        || start_ent + (sizeInBlocks / theMemoryManager->_mmBlockLength) < theMemoryManager->_mmList) return;

	if(theMemoryManager->_mmBlockLength > sizeInBytes)
	{
		start_ent->type = 1;
		struct MemoryManagerEntry* current_ent = start_ent->firstContig;
		while(current_ent != start_ent)
		{
			current_ent->lastContig = start_ent - 1;
			current_ent++;
		}
		while(current_ent != theMemoryManager->_mmListEnd)
		{
			current_ent->firstContig = start_ent + 1;
			current_ent++;
		}
		start_ent->firstContig = start_ent;
		start_ent->lastContig = start_ent;
		return;
	}

	struct MemoryManagerEntry* begin = start_ent;

	//Keep reserving blocks until the requested amount is reserved
	size_t remainingBytes = sizeInBytes - theMemoryManager->_mmBlockLength;
	while(remainingBytes > 0)
	{
		start_ent->type = 1;
		start_ent++;
		remainingBytes -= theMemoryManager->_mmBlockLength;
		if(start_ent == theMemoryManager->_mmListEnd)
		{
			start_ent->type = 1;
			break;
		}
	}
	start_ent->type = 1;

	struct MemoryManagerEntry* end = start_ent;
	struct MemoryManagerEntry* current_ent = start_ent->firstContig;
	while(current_ent != begin)
	{
		current_ent->lastContig = begin - 1;
		current_ent++;
	}
	while(current_ent != end + 1)
	{
		current_ent->firstContig = begin;
		current_ent->lastContig = end;
		current_ent++;
	}
	while(current_ent != theMemoryManager->_mmListEnd && current_ent != theMemoryManager->_mmListEnd + 1)
	{
		current_ent->firstContig = start_ent + 1;
		current_ent++;
	}
}

uintptr_t mm_allocate(size_t sizeInBytes)
{
	//Ensure the sizeInBytes is valid
	if (sizeInBytes < 0) return (uintptr_t)NULL;

	int sizeInBlocks = sizeInBytes / theMemoryManager->_mmBlockLength;
	struct MemoryManagerEntry* start_ent = theMemoryManager->_allocator(sizeInBytes, theMemoryManager->_mmList);

	if (start_ent == NULL)
	{
		char output[100] = "";
		char tempStr[16] = "";
		hex_to_ascii(sizeInBytes, tempStr);
		strcat("Unable to allocate a request for ", tempStr, output);
		appendstr(output, " bytes.");
		log(3, output);
		return (uintptr_t)NULL;
	}

	uintptr_t retAddress = (uintptr_t)((void*)start_ent - (void*)theMemoryManager->_mmList) / sizeof(*start_ent) * theMemoryManager->_mmBlockLength + theMemoryManager->_memStart;

	if(theMemoryManager->_mmBlockLength > sizeInBytes)
	{
		start_ent->type = 1;
		struct MemoryManagerEntry* current_ent = start_ent->firstContig;
		while(current_ent != start_ent)
		{
			current_ent->lastContig = start_ent - 1;
			current_ent++;
		}
		while(current_ent != theMemoryManager->_mmListEnd)
		{
			current_ent->firstContig = start_ent + 1;
			current_ent++;
		}
		start_ent->firstContig = start_ent;
		start_ent->lastContig = start_ent;

		return retAddress;
	}

	struct MemoryManagerEntry* begin = start_ent;

	//Keep reserving blocks until the requested amount is reserved
	size_t remainingBytes = sizeInBytes;

	while(remainingBytes > theMemoryManager->_mmBlockLength)
	{
		start_ent->type = 1;
		start_ent++;
		remainingBytes -= theMemoryManager->_mmBlockLength;
		if(start_ent == theMemoryManager->_mmListEnd)
		{
			start_ent->type = 1;
			break;
		}
	}

	start_ent->type = 1;

	struct MemoryManagerEntry* end = start_ent;

	struct MemoryManagerEntry* current_ent = start_ent->firstContig;
	while(current_ent != begin)
	{
		current_ent->lastContig = begin - 1;
		current_ent++;
	}
	while(current_ent != end + 1)
	{
		current_ent->firstContig = begin;
		current_ent->lastContig = end;
		current_ent++;
	}
	while(current_ent != theMemoryManager->_mmListEnd && current_ent != theMemoryManager->_mmListEnd + 1)
	{
		current_ent->firstContig = start_ent + 1;
		current_ent++;
	}

	return retAddress;
}

int mm_free(uintptr_t address)
{
	struct MemoryManagerEntry* start_ent = theMemoryManager->_mmList;
	start_ent += (address - theMemoryManager->_memStart) / theMemoryManager->_mmBlockLength;

	if(address < theMemoryManager->_memStart || address > theMemoryManager->_memStart + theMemoryManager->_mmNumEntries * theMemoryManager->_mmBlockLength || start_ent->firstContig != start_ent)
	{
		log(3, "Attempted to free memory at an invalid address. Ignoring...");
		return -1;
	}
	if(start_ent->type == 0)
	{
		log(3, "Attempted to free memory that has already been freed. Ignoring...");
		return -1;
	}

	struct MemoryManagerEntry* current_ent = start_ent;
	while(current_ent != start_ent->lastContig + 1)
	{
		current_ent->type = 0;
		current_ent++;
	}

	struct MemoryManagerEntry* leftContig = start_ent->firstContig;
	struct MemoryManagerEntry* rightContig = start_ent->lastContig;
	if(start_ent != theMemoryManager->_mmList && (start_ent - 1)->type == 0) leftContig = (start_ent - 1)->firstContig;
	if(current_ent != theMemoryManager->_mmListEnd + 1 && (current_ent + 1)->type == 0) rightContig = (current_ent + 1)->lastContig;

	struct MemoryManagerEntry* pastEndOfOriginalAlloc = start_ent->lastContig + 1;

	if(leftContig != start_ent->firstContig)
	{
		current_ent = leftContig;
		while(current_ent != start_ent)
		{
			current_ent->lastContig = rightContig;
			current_ent++;
		}
	}
	if(rightContig != start_ent->lastContig)
	{
		current_ent = pastEndOfOriginalAlloc;
		while(current_ent != pastEndOfOriginalAlloc->lastContig)
		{
			current_ent->firstContig = leftContig;
			current_ent++;
		}
		current_ent->firstContig = leftContig;
	}
	if(leftContig != start_ent->firstContig || rightContig != start_ent->lastContig)
	{
		current_ent = start_ent;
		while(current_ent != pastEndOfOriginalAlloc)
		{
			if(leftContig != start_ent->firstContig)
			{
				current_ent->firstContig = leftContig;
			}
			if(rightContig != start_ent->lastContig)
			{
				current_ent->lastContig = rightContig;
			}
			current_ent++;
		}
	}
	return 0;
}

void mm_setAllocator(struct MemoryManagerEntry* (*allocator)(size_t, struct MemoryManagerEntry*))
{
	theMemoryManager->_allocator = allocator;
}

struct MemoryManagerEntry* mm_getList()
{
	return theMemoryManager->_mmList;
}

unsigned mm_getWordSize()
{
	return theMemoryManager->_wordSize;
}

uintptr_t mm_getMemoryStart()
{
	return theMemoryManager->_memStart;
}

unsigned mm_getMemoryLimit()
{
	return (unsigned)theMemoryManager->_memStart + (unsigned)theMemoryManager->_wordSize;
}

void mm_logblock(int blocknum)
{
	if(blocknum < 0 || blocknum > theMemoryManager->_mmNumEntries)
	{
		log(3, "Attempted to retrieve information on a non-existent memory block. Ignoring...");
		return;
	}

	if (theMemoryManager->_memStart == (uintptr_t)NULL)
    {
        log(4, "Attempted to log a memory block before initializing the memory manager. Ignoring...");
        return;
    }

	struct MemoryManagerEntry* current_ent = theMemoryManager->_mmList;

	current_ent += blocknum;

	char output[100] = "";
	char tempStr[16] = "";
	char tempStr2[16] = "";
	char tempStr3[16] = "";
	hex_to_ascii(theMemoryManager->_mmBlockLength * blocknum + theMemoryManager->_memStart, tempStr);
	int_to_ascii(blocknum, tempStr2);
	hex_to_ascii(theMemoryManager->_mmBlockLength, tempStr3);

	strcat("Memory Block ", tempStr2, output);
	appendstr(output, " - start: ");
	appendstr(output, tempStr);
	appendstr(output, " length (bytes): ");
	appendstr(output, tempStr3);
	if(current_ent->type == 0) appendstr(output, " type: (AVAIL)");
	else appendstr(output, " type: (RESRV)");

	log(1, output);
}

void mm_copy(uintptr_t src, uintptr_t dest, size_t len) 
{
	uint8_t* psrc = (uint8_t*)src;
	uint8_t* pdest = (uint8_t*)dest;
    for (uint32_t i = 0; i < len; i++) {
        *(pdest + i) = *(psrc + i);
    }
}

void mm_set(uintptr_t dest, uint8_t val, size_t len)
{
    uint8_t* temp = (uint8_t*)dest;
    for (; len != 0; len--) *temp++ = val;
}

void prepare_memory_manager(struct MemoryMapEntry* mmap, size_t mmap_size)
{
	if(theMemoryManager != NULL)
	{
		log(6, "prepare_memory_manager was called multiple times! Ignoring subsequent calls...");
		return;
	}

    uintptr_t memoryStart = (uintptr_t)NULL;
    uintptr_t memoryEnd = (uintptr_t)NULL;

	uintptr_t mmStart = (uintptr_t)NULL;
	uintptr_t mmEnd = (uintptr_t)NULL;

	uintptr_t largestMemoryBlock = (uintptr_t)NULL;
	uint64_t sizeOfLargestMemoryBlock = 0;
	struct MemoryMapEntry* currentEntry = mmap;
	log(1, "Physical Memory Map");
	for(uint32_t i = 0; i < mmap_size; i++)
	{
		char output[100] = "";
		char tempStr[10] = "";
		char tempStr2[10] = "";
		char tempStr3[10] = "";
		char tempStr4[10] = "";

		int_to_ascii(i, tempStr);
		strcat("Region ", tempStr, output);
		appendstr(output, " - start: ");
		hex_to_ascii(currentEntry->baseAddress, tempStr2);
		appendstr(output, tempStr2);
		appendstr(output, " length (bytes): ");
		hex_to_ascii(currentEntry->length, tempStr3);
		appendstr(output, tempStr3);
		appendstr(output, " type: ");
		appendstr(output, " (");

		if(currentEntry->type == 1) {
			appendstr(output, "AVAIL");
		}
		else if(currentEntry->type == 2) {
			appendstr(output, "RESRV");
		}
		else if(currentEntry->type == 3) {
			appendstr(output, "ACPIR");
		}
		else if(currentEntry->type == 4) {
			appendstr(output, "ACPIN");
		}
		else {
			appendstr(output, "UNKWN");
		}
		appendstr(output, ")");
		log(1, output);

        if(i == mmap_size - 1)
		{
            memoryEnd = (uintptr_t)(currentEntry->baseAddress - 1 + currentEntry->length);
        }

        if(currentEntry->length > sizeOfLargestMemoryBlock && currentEntry->type == 1 && currentEntry->baseAddress >= 0x90000)
		{
			sizeOfLargestMemoryBlock = currentEntry->length;
			largestMemoryBlock = (uintptr_t)currentEntry->baseAddress;
		}

		currentEntry++;
	}

	if(largestMemoryBlock != (uintptr_t)NULL)
	{
		mmStart = largestMemoryBlock;
		struct MemoryManagerEntry test;
		mmEnd = (uintptr_t)sizeOfLargestMemoryBlock / (sizeof(test) + 64) * sizeof(test) - 1;
		memoryStart = mmEnd + 1;
	}

	if(mmStart == (uintptr_t)NULL || mmEnd == (uintptr_t)NULL)
	{
		log(6, "Failed to find a suitable memory location to initialize the memory manager! Memory manipulation is unavailable.");
		return;
	}

	create_memory_manager(1, &bestFit, mmStart, mmEnd - mmStart);
	mm_initialize((size_t)(memoryEnd-memoryStart), memoryStart);

	char output[100] = "";
	char memStartString[16] = "";
	hex_to_ascii(theMemoryManager->_memStart, memStartString);
	strcat("Initializing memory space starting at ", memStartString, output);
	appendstr(output, ".");
	log(1, output);

    currentEntry = mmap;
    for(uint32_t i = 0; i < mmap_size; i++) {
        if(currentEntry->type == 1 || currentEntry->baseAddress + currentEntry->length <= memoryStart)
		{
			currentEntry++;
			continue;
		}

        uintptr_t resrvStart = currentEntry->baseAddress;
        uintptr_t resrvLength = currentEntry->length;
        if(currentEntry->baseAddress < memoryStart) {
            resrvStart = memoryStart;
            resrvLength = currentEntry->length - (memoryStart - currentEntry->baseAddress);
        }

        mm_reserveat(resrvLength, resrvStart);

        currentEntry++;
    }
}

//Memory allocation method
struct MemoryManagerEntry* bestFit(size_t sizeInBytes, struct MemoryManagerEntry* list)
{
	size_t sizeInBlocks = sizeInBytes / theMemoryManager->_mmBlockLength;

	struct MemoryManagerEntry* smallestLargeEnoughHole = NULL;
	size_t smallestLargeEnoughHoleSize = 0;

	struct MemoryManagerEntry* current_ent = theMemoryManager->_mmList;

	while(current_ent != theMemoryManager->_mmListEnd && current_ent != theMemoryManager->_mmListEnd + 1)
	{
		if (current_ent->type != 0)
		{
			current_ent = current_ent->lastContig + 1;
			continue;
		}

		size_t holeSize = (uintptr_t)((void*)current_ent->lastContig - (void*)current_ent->firstContig + (void*)sizeof(*current_ent)) / sizeof(*current_ent) * theMemoryManager->_mmBlockLength;

		if(holeSize >= sizeInBytes)
		{
			if (holeSize < smallestLargeEnoughHoleSize || smallestLargeEnoughHoleSize == 0)
			{
				smallestLargeEnoughHoleSize = holeSize;
				smallestLargeEnoughHole = current_ent;
			}
		}

		current_ent = current_ent->lastContig + 1;
	}
	return smallestLargeEnoughHole;
}
