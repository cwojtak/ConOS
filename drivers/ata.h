#ifndef DISK_H
#define DISK_H

#include "../cpu/ports.h"

int ata_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);

#endif
