#ifndef DISK_H
#define DISK_H

#include <stdint.h>

enum FS_ERROR {
    OK,
    FILE_NOT_FOUND,
    FILE_EXISTS,
    INVALID_PATH,
    NOT_A_DIRECTORY,
    NOT_A_FILE,
    FS_UNINITIALIZED,
    FS_INSUFFICIENT_SPACE
};

void set_disk_reader(int (*srsfd)(uint32_t, uint8_t, uintptr_t));
int read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);

void set_disk_writer(int (*swstd)(uint32_t, uint16_t, uint16_t, uintptr_t));
int write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf);

#endif