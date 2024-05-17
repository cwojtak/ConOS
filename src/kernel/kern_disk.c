#include "kern_disk.h"

void prepare_kernel_fs(Array* pci_devices)
{
    log(1, "Loading file system...");

    int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t) = NULL;

    for(int i = 0; i < pci_devices->used; i++)
    {
        uintptr_t addr = pci_devices->array + (uintptr_t)i * sizeof(uintptr_t);
		uintptr_t struc = *(uintptr_t*)(addr);
		struct PCI_DEVICE* device = (struct PCI_DEVICE*)struc;
        if(device->baseClass == 0x1 && device->subClass == 0x1 && device->progIF == 0x0)
        {
            read_sectors_from_disk = ata_read_sectors_from_disk;
            break;
        }
    }

    if(read_sectors_from_disk == NULL)
    {
        log(3, "No storage devices supported by this OS were found. Failed to load the file system.");
        return;
    }

    uintptr_t mbr = load_mbr(read_sectors_from_disk);
    uintptr_t fat = load_fat((struct mbr_info*)mbr, read_sectors_from_disk);
    uintptr_t root_directory = load_root_directory((struct mbr_info*)mbr, read_sectors_from_disk);
    fat12_initialize_info((struct mbr_info*)mbr, fat, root_directory);
    log(1, "File system successfully loaded!");
}

uintptr_t load_mbr(int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t))
{
    uintptr_t buf = mm_allocate(0x200);
    read_sectors_from_disk(0, 1, buf);
    return buf;
}

uintptr_t load_fat(struct mbr_info* mbr, int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t))
{
    uint32_t fat_size = mbr->fatSize;
    uint32_t fat_offset = mbr->reservedSectorCount;
    uintptr_t buf = mm_allocate(fat_size * mbr->bytesPerSector);
    
    read_sectors_from_disk(fat_offset, fat_size, buf);

    return buf;
}

uintptr_t load_root_directory(struct mbr_info* mbr, int (*read_sectors_from_disk)(uint32_t, uint8_t, uintptr_t))
{
    uint32_t fat_size = mbr->fatSize;
    uint32_t fat_offset = mbr->reservedSectorCount;
    uint32_t num_fats = mbr->numFats;

    uint32_t root_size = mbr->rootEntryCount * 32 / mbr->bytesPerSector;
    uint32_t root_offset = fat_offset + (num_fats * fat_size);
    uintptr_t buf = mm_allocate(root_size * mbr->bytesPerSector);

    read_sectors_from_disk(root_offset, root_size, buf);

    return buf;
}
