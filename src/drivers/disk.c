#include "disk.h"

static int (*selected_read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t);

void set_disk_reader(int (*srsfd)(uint32_t, uint8_t, uintptr_t))
{
    selected_read_sectors_from_disk = srsfd;
}

int read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf)
{
    selected_read_sectors_from_disk(lba_sector, num_sectors, buf);
}