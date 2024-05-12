#ifndef KERN_DISK
#define KERN_DISK

#include "../drivers/ata.h"
#include "../drivers/fat12.h"
#include "kern_mem.h"
#include "log.h"

void prepare_kernel_fs();
uintptr_t load_mbr();
uintptr_t load_fat(struct mbr_info* mbr);
uintptr_t load_root_directory(struct mbr_info* mbr);

#endif
