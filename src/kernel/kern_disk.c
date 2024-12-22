#include "kern_disk.h"

static struct FS_FUNCTIONS fs_functions;

void prepare_kernel_fs(Array* pci_devices)
{
    log(1, "Loading file system...");

    char setDiskReader = 0;

    for(int i = 0; i < pci_devices->used; i++)
    {
        uintptr_t addr = pci_devices->array + (uintptr_t)i * sizeof(uintptr_t);
		uintptr_t struc = *(uintptr_t*)(addr);
		struct PCI_DEVICE* device = (struct PCI_DEVICE*)struc;
        if(device->baseClass == 0x1 && device->subClass == 0x1 && device->progIF == 0x0)
        {
            setDiskReader = 1;
            set_disk_reader(ata_read_sectors_from_disk);
            set_disk_writer(ata_write_sectors_to_disk);
            break;
        }
    }

    if(!setDiskReader)
    {
        log(3, "No storage devices supported by this OS were found. Failed to load the file system.");
        return;
    }

    //Identify the filesystem
    //First, trying loading an MBR to identify FAT filesystems
    uintptr_t mbr = load_mbr();
    struct fat12_mbr_info* common_mbr = (struct fat12_mbr_info*)mbr;
    
    //Sanity check to ensure we are loading some sort of FAT filesystem
    if(common_mbr->fatType[0] == 'F' && common_mbr->fatType[1] == 'A' && common_mbr->fatType[2] == 'T')
    {
        //Determine the total number of sectors of the filesystem
        uint32_t totalSectors = 0;
        if(common_mbr->totalSectors16 != 0) totalSectors = common_mbr->totalSectors16;
        else totalSectors = common_mbr->totalSectors32;

        //Determine the number of clusters
        uint32_t totalClusters = (totalSectors - (common_mbr->reservedSectorCount + (common_mbr->numFats * common_mbr->fatSize) +
            (((common_mbr->rootEntryCount * 32) + common_mbr->bytesPerSector - 1) / common_mbr->bytesPerSector))) / common_mbr->sectorsPerCluster;

        //FAT 12
        if(totalClusters < 4085)
        {
            uintptr_t fat = load_fat((struct fat12_mbr_info*)mbr);
            uintptr_t root_directory = load_root_directory((struct fat12_mbr_info*)mbr);
            fat12_initialize_info((struct fat12_mbr_info*)mbr, fat, root_directory);

            fs_functions.fs_format = FAT12;
            fs_functions.get_total_space = fat12_get_total_space;
            fs_functions.get_free_space = fat12_get_free_space;
            fs_functions.enumerate_files = fat12_enumerate_files;
            fs_functions.find_file = fat12_find_file;
            fs_functions.load_file = fat12_load_file;
            log(1, "File system successfully loaded!");
            return;
        }
        //FAT 16
        else if (totalClusters < 65525)
        {
            log(3, "Detected a FAT 16 file system; unable to load this type of file system.");
            return;
        }
        //FAT 32
        else
        {
            log(3, "Detected a FAT 32 file system; unable to load this type of file system.");
            return;
        }
    }

    log(3, "Unable to detect the file system type of the current disk. No file system will be loaded.");
}

struct FS_FUNCTIONS* fs_get_functions()
{
    return &fs_functions;
}

uintptr_t load_mbr()
{
    uintptr_t buf = mm_allocate(0x200);
    read_sectors_from_disk(0, 1, buf);
    return buf;
}

uintptr_t load_fat(struct fat12_mbr_info* mbr)
{
    uint32_t fat_size = mbr->fatSize;
    uint32_t fat_offset = mbr->reservedSectorCount;
    uintptr_t buf = mm_allocate(fat_size * mbr->bytesPerSector);
    
    read_sectors_from_disk(fat_offset, fat_size, buf);

    return buf;
}

uintptr_t load_root_directory(struct fat12_mbr_info* mbr)
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
