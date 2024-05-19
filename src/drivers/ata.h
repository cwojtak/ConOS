#ifndef ATA_H
#define ATA_H

#include "../cpu/ports.h"
#include "../kernel/log.h"

int ata_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);

#endif
