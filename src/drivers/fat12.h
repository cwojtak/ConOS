#ifndef FAT12_H
#define FAT12_H

#include "../kernel/kern_mem.h"
#include "../libc/dynamic_array.h"
#include "disk.h"

#define MAX_FILENAME_SIZE 261 //Max file name size (with null character)
#define MAX_PATH_SIZE 32768 //Max file name size (with null character)

struct FILE {
    char path[32768];
    uint8_t isDirectory;
    uint16_t firstCluster;
    uint32_t fileSize;
};

struct FILE_ENUMERATION {
    struct FILE* files;
    uint32_t numFiles;
};

struct fat12_fs_info {
    struct fat12_mbr_info* mbr;
    uintptr_t fat;
    uintptr_t rootDir;
    uint64_t bytesFree;
    uint64_t bytesTotal;
};

struct __attribute__((__packed__)) fat12_mbr_info {
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

void fat12_initialize_info(struct fat12_mbr_info* mbr, uintptr_t fat, uintptr_t rootDir);
uint64_t fat12_get_total_space();
uint64_t fat12_get_free_space();
uint16_t fat12_get_cluster(uint32_t clusterNumber);
enum FS_ERROR fat12_enumerate_files(struct FILE* directory, struct FILE_ENUMERATION* out);
enum FS_ERROR fat12_find_file(char path[], struct FILE* output);
enum FS_ERROR fat12_load_file(struct FILE* file, void** buf, uint64_t* bytesRead);
enum FS_ERROR fat12_write_file(char path[], void** buf, uint32_t bytesToWrite);

uint8_t fat12_checksum(char* shortName);

#endif
