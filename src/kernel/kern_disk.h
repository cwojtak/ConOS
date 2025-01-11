#ifndef KERN_DISK_H
#define KERN_DISK_H

#include "../drivers/ata.h"
#include "../drivers/disk.h"
#include "../drivers/fat12.h"
#include "kern_mem.h"
#include "log.h"
#include "../libc/dynamic_array.h"
#include "kern_pci.h"


enum FS_TYPE
{
    FAT12,
    FAT16,
    FAT32
};

struct FS_FUNCTIONS
{
    enum FS_TYPE fs_format;
    uint64_t (*get_total_space)();
    uint64_t (*get_free_space)();
    enum FS_ERROR (*enumerate_files)(struct FILE* directory, struct FILE_ENUMERATION* out);
    enum FS_ERROR (*find_file)(char path[], struct FILE* output);
    enum FS_ERROR (*load_file)(struct FILE* file, void** buf, uint64_t* bytesRead);
    enum FS_ERROR (*write_file)(char path[], void** buf, uint32_t bytesToWrite);
};

void prepare_kernel_fs(Array* pci_devices);
struct FS_FUNCTIONS* fs_get_functions();

uintptr_t load_mbr();
uintptr_t load_fat(struct fat12_mbr_info* mbr);
uintptr_t load_root_directory(struct fat12_mbr_info* mbr);

#endif
