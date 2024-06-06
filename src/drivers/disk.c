#include "disk.h"

static int (*selected_read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t);
static int (*selected_write_sectors_to_disk)(uint32_t, uint16_t, uint16_t, uintptr_t);

void set_disk_reader(int (*srsfd)(uint32_t, uint8_t, uintptr_t))
{
    selected_read_sectors_from_disk = srsfd;
}

int read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf)
{
    selected_read_sectors_from_disk(lba_sector, num_sectors, buf);
}

void set_disk_writer(int (*swstd)(uint32_t, uint16_t, uint16_t, uintptr_t))
{
    selected_write_sectors_to_disk = swstd;
}

int write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf)
{
    selected_write_sectors_to_disk(lba_sector, offset, length, buf);
}