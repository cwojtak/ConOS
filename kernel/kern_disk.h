#ifndef KERN_DISK
#define KERN_DISK

#include "../drivers/ata.h"
#include "kern_mem.h"
#include "log.h"

struct __attribute__((__packed__)) mbr_info {
    uint8_t reserved1;
    uint16_t reserved2;
    char OEMName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t numFats;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t media;
    uint16_t fatSize;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    uint8_t driveNumber;
    uint8_t reserved3;
    uint8_t bootSignature;
    uint32_t volumeID;
    char volumeLabel[11];
    char fatType[8];
};

uintptr_t load_mbr();
uintptr_t load_fat(struct mbr_info* mbr);
uintptr_t load_root_directory(struct mbr_info* mbr);

#endif
