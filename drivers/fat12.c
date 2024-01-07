#include "fat12.h"

static struct fat12_fs_info* fs_info;

void fat12_initialize_info(struct mbr_info* mbr, uintptr_t fat, uintptr_t rootDir)
{
    fs_info = (struct fat12_fs_info*)mm_allocate(sizeof(uintptr_t) * 2 + sizeof(struct mbr_info*));
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
            if(entry[0] == 0x0) continue;
            
            numFiles++;
            char fileName[11];
            for(int i = 0; i < 11; i++) fileName[i] = entry[i];
            if(strlen(directory->path) > 244)
            {
                log(4, "File enumeration failed; file path too long (> 256 characters).");
                return;
            }
            strcat(directory->path, fileName, f_enum[i].path);
            f_enum[i].isDirectory = 0; //TODO: update to detect subdirectories
            f_enum[i].firstCluster = *(((uint16_t*)entry) + 13);
        }
    }
    else
    {
        //TODO: implement enumeration for subdirectories
    }
    out->files = f_enum;
    out->numFiles = numFiles;
}

uintptr_t fat12_load_file(struct FILE* file)
{
}
