#include "kern_mem.h"

struct MemoryManager* theMemoryManager;

void create_memory_manager(unsigned wordSize, int (*allocator)(int, uintptr_t), uintptr_t mmStart, size_t objectLength)
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
}

void mm_logblock(int blocknum)
{
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

void prepare_memory_manager(struct MemoryMapEntry* mmap, size_t mmap_size)
{
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

        if(currentEntry->length > sizeOfLargestMemoryBlock && currentEntry->type == 1)
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

//Memory allocation methods
int bestFit(int sizeInWords, uintptr_t list)
{
	uint16_t* holes = (uint16_t*)list;
	int16_t smallestLargeEnoughHole = -1;
	int16_t smallestLargeEnoughHoleSize = -1;
	for (int i = 1; i < holes[0] * 2 + 1; i += 2)
	{
		if (holes[i + 1] >= sizeInWords)
		{
			if (holes[i + 1] < smallestLargeEnoughHoleSize || smallestLargeEnoughHoleSize == -1)
			{
				smallestLargeEnoughHole = holes[i];
				smallestLargeEnoughHoleSize = holes[i + 1];
			}
		}
	}
	return smallestLargeEnoughHole;
}

int worstFit(int sizeInWords, uintptr_t list)
{
	uint16_t* holes = (uint16_t*)list;
	int16_t largestPossibleHole = -1;
	int16_t largestPossibleHoleSize = -1;
	for (int i = 1; i < holes[0] * 2 + 1; i += 2)
	{
		if (holes[i + 1] >= sizeInWords)
		{
			if (holes[i + 1] > largestPossibleHole || largestPossibleHoleSize == -1)
			{
				largestPossibleHole = holes[i];
				largestPossibleHoleSize = holes[i + 1];
			}
		}
	}
	return largestPossibleHole;
}
