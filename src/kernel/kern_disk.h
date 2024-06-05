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

void prepare_kernel_fs(Array* pci_devices);
uintptr_t load_mbr();
uintptr_t load_fat(struct fat12_mbr_info* mbr);
uintptr_t load_root_directory(struct fat12_mbr_info* mbr);

#endif
