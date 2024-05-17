#ifndef FAT12_H
#define FAT12_H

#include "../kernel/kern_mem.h"

struct FILE {
    char path[256];
    uint8_t isDirectory;
    uint16_t firstCluster;
    uint32_t fileSize;
};

struct FILE_ENUMERATION {
    struct FILE* files;
    uint32_t numFiles;
};

struct fat12_fs_info {
    struct mbr_info* mbr;
    uintptr_t fat;
    uintptr_t rootDir;
};

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

void fat12_initialize_info(struct mbr_info* mbr, uintptr_t fat, uintptr_t rootDir);
void fat12_enumerate_files(struct FILE* directory, struct FILE_ENUMERATION* out);
uintptr_t fat12_load_file(struct FILE* file);

#endif
