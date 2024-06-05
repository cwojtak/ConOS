#ifndef DISK_H
#define DISK_H

#include <stdint.h>

enum FS_ERROR {
    OK,
    FILE_NOT_FOUND,
    FILE_EXISTS,
    INVALID_PATH,
    NOT_A_DIRECTORY,
    NOT_A_FILE
};

void set_disk_reader(int (*srsfd)(uint32_t, uint8_t, uintptr_t));
int read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);

#endif