#include "fat12.h"

static struct fat12_fs_info* fs_info;

void fat12_initialize_info(struct mbr_info* mbr, uintptr_t fat, uintptr_t rootDir)
{
    fs_info = (struct fat12_fs_info*)mm_allocate(sizeof(struct fat12_fs_info));
    fs_info->mbr = mbr;
    fs_info->fat = fat;
    fs_info->rootDir = rootDir;
}

void fat12_enumerate_files(struct FILE* directory, struct FILE_ENUMERATION* out)
{
    struct FILE* f_enum = (struct FILE*)mm_allocate(sizeof(struct FILE) * fs_info->mbr->rootEntryCount);
    uint32_t numFiles = 0;
    if(strcmp(directory->path, "/") == 0)
    {
        for(int i = 0; i < fs_info->mbr->rootEntryCount; i++)
        {
            char* entry = (char*)(fs_info->rootDir) + (i * 32);
            if((entry[11] & 0x8) != 0 || (entry[11] & 0x4) != 0) continue;
            if(entry[0] == 0x0) break;
            
            char fileName[12];
            for(int j = 0; j < 11; j++) fileName[j] = entry[j];
            fileName[11] = '\0';
            if(strlen(directory->path) > 244)
            {
                log(4, "File enumeration failed; file path too long (> 256 characters).");
                return;
            }
            strcat(directory->path, fileName, f_enum[numFiles].path);
            strcat(f_enum[numFiles].path, "\0", f_enum[numFiles].path);
            f_enum[numFiles].isDirectory = 0; //TODO: update to detect subdirectories
            f_enum[numFiles].firstCluster = *(((uint16_t*)entry) + 13);
            f_enum[numFiles].fileSize = (entry[28] & 0xFF) | ((entry[29] & 0xFF) << 8) | ((entry[30] & 0xFF) << 16) | ((entry[31] & 0xFF) << 24);
            numFiles++;
        }
    }
    else
    {
        //TODO: implement enumeration for subdirectories
    }
    out->files = f_enum;
    out->numFiles = numFiles;
}

uint64_t fat12_load_file(struct FILE* file, void** buf)
{
    if(file->isDirectory != 0) return 0;
    uint32_t numSectors = 0;
    uint16_t currentCluster = file->firstCluster;
    while(currentCluster != 0x0FFF)
    {
        uintptr_t fat_entry_offset = fs_info->fat;
        fat_entry_offset += currentCluster + currentCluster / 2;
        uint16_t clusterEntry = 0;
        if (currentCluster % 2 == 0)
            clusterEntry = (*((uint16_t*)fat_entry_offset) & 0x0FFF);
        else
            clusterEntry = (*((uint16_t*)fat_entry_offset) >> 4);
        
        if(clusterEntry == 0x0 || (clusterEntry > 0xFF7 && clusterEntry != 0xFFF)) return 0;

        numSectors++;
        currentCluster = clusterEntry;
    }

    *buf = (void*)mm_allocate(numSectors * 512);
    currentCluster = file->firstCluster;
    uint32_t i = 0;
    while(currentCluster != 0x0FFF)
    {
        uint32_t data_offset = fs_info->mbr->reservedSectorCount + (fs_info->mbr->numFats * fs_info->mbr->fatSize) + (fs_info->mbr->rootEntryCount * 32 / fs_info->mbr->bytesPerSector);
        data_offset += currentCluster - 2;
        read_sectors_from_disk(data_offset, 1, ((uintptr_t)(*buf)) + i * 512);

        uintptr_t fat_entry_offset = fs_info->fat;
        fat_entry_offset += currentCluster + currentCluster / 2;
        uint16_t clusterEntry = 0;
        if (currentCluster % 2 == 0)
            clusterEntry = (*((uint16_t*)fat_entry_offset) & 0x0FFF);
        else
            clusterEntry = (*((uint16_t*)fat_entry_offset) >> 4);

        currentCluster = clusterEntry;
        i++;
    }

    return numSectors * 512;
}
