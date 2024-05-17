#ifndef KERN_DISK
#define KERN_DISK

#include "../drivers/ata.h"
#include "../drivers/fat12.h"
#include "kern_mem.h"
#include "log.h"
#include "../libc/dynamic_array.h"
#include "kern_pci.h"

void prepare_kernel_fs(Array* pci_devices);
uintptr_t load_mbr(int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t));
uintptr_t load_fat(struct mbr_info* mbr, int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t));
uintptr_t load_root_directory(struct mbr_info* mbr, int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t));

#endif
