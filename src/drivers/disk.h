#ifndef DISK_H
#define DISK_H

#include <stdint.h>

void set_disk_reader(int (*srsfd)(uint32_t, uint8_t, uintptr_t));
int read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);

#endif