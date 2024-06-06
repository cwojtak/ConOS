#ifndef ATA_H
#define ATA_H

#include "../cpu/ports.h"
#include "../kernel/log.h"

int ata_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);
int ata_write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf);

#endif
