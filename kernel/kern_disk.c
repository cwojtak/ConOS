#include "kern_disk.h"

uintptr_t load_mbr()
{
    uintptr_t buf = mm_allocate(0x200);
    ata_read_sectors_from_disk(0, 1, buf);
    return buf;
}

uintptr_t load_fat(struct mbr_info* mbr)
{
    uint32_t fat_size = mbr->fatSize;
    uint32_t fat_offset = mbr->reservedSectorCount;
    uintptr_t buf = mm_allocate(fat_size * mbr->bytesPerSector);
    ata_read_sectors_from_disk(fat_offset, fat_size, buf);
    return buf;
}

uintptr_t load_root_directory(struct mbr_info* mbr)
{
    uint32_t fat_size = mbr->fatSize;
    uint32_t fat_offset = mbr->reservedSectorCount;
    uint32_t num_fats = mbr->numFats;

    uint32_t root_size = mbr->rootEntryCount * 32 / mbr->bytesPerSector;
    uint32_t root_offset = fat_offset + (num_fats * fat_size);
    uintptr_t buf = mm_allocate(root_size * mbr->bytesPerSector);
    ata_read_sectors_from_disk(root_offset, root_size, buf);

    return 0;
}
