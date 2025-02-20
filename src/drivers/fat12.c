#include "fat12.h"

static struct fat12_fs_info* fs_info = (struct fat12_fs_info*)NULL;

void fat12_initialize_info(struct fat12_mbr_info* mbr, uintptr_t fat, uintptr_t rootDir)
{
    if(fs_info != NULL)
    {
        mm_free((uintptr_t)fs_info->mbr);
        mm_free((uintptr_t)fs_info->fat);
        mm_free((uintptr_t)fs_info->rootDir);
        mm_free((uintptr_t)fs_info);
    }
    fs_info = (struct fat12_fs_info*)mm_allocate(sizeof(struct fat12_fs_info));
    fs_info->mbr = mbr;
    fs_info->fat = fat;
    fs_info->rootDir = rootDir;

    // Record available space on disk
    //Determine the total number of sectors of the filesystem
    uint32_t totalSectors = 0;
    if(fs_info->mbr->totalSectors16 != 0) totalSectors = fs_info->mbr->totalSectors16;
    else totalSectors = fs_info->mbr->totalSectors32;

    //Determine the number of clusters
    uint32_t totalClusters = (totalSectors - (fs_info->mbr->reservedSectorCount + (fs_info->mbr->numFats * fs_info->mbr->fatSize) +
        (((fs_info->mbr->rootEntryCount * 32) + fs_info->mbr->bytesPerSector - 1) / fs_info->mbr->bytesPerSector))) / fs_info->mbr->sectorsPerCluster;
    
    fs_info->bytesTotal = (totalClusters) * fs_info->mbr->bytesPerSector;
    fs_info->bytesFree = 0;

    for(uint32_t i = 2; i < totalClusters + 2; i++)
    {
        if(fat12_get_cluster(i) == 0)
            fs_info->bytesFree += 512;
    }
}

uint64_t fat12_get_total_space()
{
    if(fs_info == NULL)
        return 0;
    else
        return fs_info->bytesTotal;
}

uint64_t fat12_get_free_space()
{
    if(fs_info == NULL)
        return 0;
    else
        return fs_info->bytesFree;
}

uint16_t fat12_get_cluster(uint32_t clusterNumber)
{
    uintptr_t fat_entry_offset = fs_info->fat;
    fat_entry_offset += clusterNumber + clusterNumber / 2;
    uint16_t clusterEntry = 0;
    if (clusterNumber % 2 == 0)
        clusterEntry = (*((uint16_t*)fat_entry_offset) & 0x0FFF);
    else
        clusterEntry = (*((uint16_t*)fat_entry_offset) >> 4);

    return clusterEntry;
}

void fat12_set_cluster(uint32_t clusterNumber, uint16_t value)
{
    uint32_t offset = clusterNumber * 1.5;

    if(clusterNumber % 2 == 0)
    {
        ((uint8_t*)(fs_info->fat))[offset] = value & 0xFF;
        ((uint8_t*)(fs_info->fat))[offset + 1] = (((uint8_t*)(fs_info->fat))[offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
    else
    {
        ((uint8_t*)(fs_info->fat))[offset] = (((uint8_t*)(fs_info->fat))[offset] & 0x0F) | ((value << 4) & 0xF0);
        ((uint8_t*)(fs_info->fat))[offset + 1] = (value >> 4) & 0xFF;
    }

    write_sectors_to_disk(fs_info->mbr->reservedSectorCount, offset, 3, (uintptr_t)&(((uint8_t*)(fs_info->fat))[offset]));
}

enum FS_ERROR fat12_enumerate_files(struct FILE* directory, struct FILE_ENUMERATION* out)
{
    if(fs_info == NULL)
        return FS_UNINITIALIZED;
    struct FILE* f_enum = (struct FILE*)mm_allocate(sizeof(struct FILE) * fs_info->mbr->rootEntryCount);
    uint32_t numFiles = 0;
    if(strcmp(directory->path, "/") == 0)
    {
        uint8_t longNameLength = 0;
        char* fileName = (char*)mm_allocate(261);
        fileName[0] = '\0';
        for(int i = 0; i < fs_info->mbr->rootEntryCount; i++)
        {
            char* entry = (char*)(fs_info->rootDir) + (i * 32);

            // Check for long file name entries
            if(entry[11] == 0x0F)
            {
                if(longNameLength < 260)
                {
                    // Obtain parts of the long name from this entry
                    char longNameFirst[10];
                    char longNameSecond[12];
                    char longNameThird[4];
                    
                    // "Convert" from Unicode to ASCII
                    unicode_to_ascii(&(entry[1]), longNameFirst, 10);
                    unicode_to_ascii(&(entry[14]), longNameSecond, 12);
                    unicode_to_ascii(&(entry[28]), longNameThird, 4);

                    // Concatenate file name together
                    char* fileNameTempBuffer = (char*)mm_allocate(MAX_FILENAME_SIZE);
                    char* begin = fileNameTempBuffer;
                    fileNameTempBuffer[13] = '\0';

                    strcpy(longNameFirst, fileNameTempBuffer);
                    fileNameTempBuffer += 5;
                    strcpy(longNameSecond, fileNameTempBuffer);
                    fileNameTempBuffer += 6;
                    strcpy(longNameThird, fileNameTempBuffer);

                    strcat(begin, fileName, begin);
                    strcpy(begin, fileName);

                    // Increment name length
                    longNameLength += 13;

                    // Free allocated memory
                    mm_free((uintptr_t)fileNameTempBuffer);

                    continue;
                }
                else
                    continue;
            }

            if((entry[11] & 0x8) != 0 || (entry[11] & 0x4) != 0) continue;
            if(entry[0] == 0x0) break;
            

            if(longNameLength == 0)
            {
                for(int j = 0; j < 11; j++) fileName[j] = entry[j];
                fileName[11] = '\0';
            }
            else
            {
                fileName[longNameLength] = '\0';
            }

            if(strlen(directory->path) > MAX_PATH_SIZE - 12)
            {
                log(4, "File enumeration failed; file path too long.");
                return INVALID_PATH;
            }
            strcat(directory->path, fileName, f_enum[numFiles].path);
            strcat(f_enum[numFiles].path, "\0", f_enum[numFiles].path);
            f_enum[numFiles].isDirectory = 0;
            f_enum[numFiles].firstCluster = *(((uint16_t*)entry) + 13);
            f_enum[numFiles].fileSize = (entry[28] & 0xFF) | ((entry[29] & 0xFF) << 8) | ((entry[30] & 0xFF) << 16) | ((entry[31] & 0xFF) << 24);
            numFiles++;

            if(longNameLength != 0)
            {
                longNameLength = 0;
                fileName[0] = '\0';
            }
        }

        // Free fileName memory
        mm_free((uintptr_t)fileName);
    }
    else
    {
        log(4, "Only the root directory is available in FAT 12.");
        return INVALID_PATH;
    }
    out->files = f_enum;
    out->numFiles = numFiles;
    return OK;
}

enum FS_ERROR fat12_find_file(char path[], struct FILE* output)
{
    if(fs_info == NULL)
        return FS_UNINITIALIZED;
    Array path_split;
    strsplit_indices(path, '/', &path_split);
    if(path_split.used != 1 || path[0] != '/')
    {
        log(3, "Invalid path provided for FAT 12 filesystem.");
        return INVALID_PATH;
    }
    uint32_t nameLocation = ((uint32_t*)path_split.array)[0];
    uintptr_t searchFileName = mm_allocate(strlen(path) - 1 - nameLocation);
    mm_copy((uintptr_t)(path + nameLocation + 1), searchFileName, strlen(path) - 1 - nameLocation);
    freeArray(&path_split);

    uint8_t longNameLength = 0;
    char* fileName = (char*)mm_allocate(261);
    fileName[0] = '\0';
    for(int i = 0; i < fs_info->mbr->rootEntryCount; i++)
    {
        char* entry = (char*)(fs_info->rootDir) + (i * 32);

        // Check for long file name entries
        if(entry[11] == 0x0F)
        {
            if(longNameLength < 260)
            {
                // Obtain parts of the long name from this entry
                char longNameFirst[10];
                char longNameSecond[12];
                char longNameThird[4];
                
                // "Convert" from Unicode to ASCII
                unicode_to_ascii(&(entry[1]), longNameFirst, 10);
                unicode_to_ascii(&(entry[14]), longNameSecond, 12);
                unicode_to_ascii(&(entry[28]), longNameThird, 4);

                // Concatenate file name together
                char* fileNameTempBuffer = (char*)mm_allocate(MAX_FILENAME_SIZE);
                char* begin = fileNameTempBuffer;
                fileNameTempBuffer[13] = '\0';

                strcpy(longNameFirst, fileNameTempBuffer);
                fileNameTempBuffer += 5;
                strcpy(longNameSecond, fileNameTempBuffer);
                fileNameTempBuffer += 6;
                strcpy(longNameThird, fileNameTempBuffer);

                strcat(begin, fileName, begin);
                strcpy(begin, fileName);

                // Increment name length
                longNameLength += 13;

                // Free allocated memory
                mm_free((uintptr_t)fileNameTempBuffer);

                continue;
            }
            else
                continue;
        }

        if((entry[11] & 0x8) != 0 || (entry[11] & 0x4) != 0) continue;
        if(entry[0] == 0x0) break;
        
        if(longNameLength == 0)
        {
            for(int j = 0; j < 11; j++) fileName[j] = entry[j];
            fileName[11] = '\0';
        }
        else
        {
            fileName[longNameLength] = '\0';
        }

        if(strcmp(fileName, (char*)searchFileName) == 0)
        {
            strcat("/", fileName, output->path);
            strcat(output->path, "\0", output->path);
            output->isDirectory = 0;
            output->firstCluster = *(((uint16_t*)entry) + 13);
            output->fileSize = (entry[28] & 0xFF) | ((entry[29] & 0xFF) << 8) | ((entry[30] & 0xFF) << 16) | ((entry[31] & 0xFF) << 24);
            return OK;
        }

        if(longNameLength != 0)
        {
            longNameLength = 0;
            fileName[0] = '\0';
        }
    }
    return FILE_NOT_FOUND;
}

enum FS_ERROR fat12_load_file(struct FILE* file, void** buf, uint64_t* bytesRead)
{
    if(fs_info == NULL)
        return FS_UNINITIALIZED;
    if(file->isDirectory != 0) return NOT_A_FILE;
    uint32_t numSectors = 0;
    uint16_t currentCluster = file->firstCluster;
    while(currentCluster != 0x0FFF)
    {
        uintptr_t fat_entry_offset = fs_info->fat;
        fat_entry_offset += currentCluster + currentCluster / 2;
        uint16_t clusterEntry = 0;
        if (currentCluster % 2 == 0)
            clusterEntry = (*((uint16_t*)fat_entry_offset) & 0x0FFF);
        else
            clusterEntry = (*((uint16_t*)fat_entry_offset) >> 4);
        
        if(clusterEntry == 0x0 || (clusterEntry > 0xFF7 && clusterEntry != 0xFFF)) return 0;

        numSectors++;
        currentCluster = clusterEntry;
    }

    *buf = (void*)mm_allocate(numSectors * 512);
    currentCluster = file->firstCluster;
    uint32_t i = 0;
    while(currentCluster != 0x0FFF)
    {
        uint32_t data_offset = fs_info->mbr->reservedSectorCount + (fs_info->mbr->numFats * fs_info->mbr->fatSize) + (fs_info->mbr->rootEntryCount * 32 / fs_info->mbr->bytesPerSector);
        data_offset += currentCluster - 2;
        read_sectors_from_disk(data_offset, 1, ((uintptr_t)(*buf)) + i * 512);

        currentCluster = fat12_get_cluster(currentCluster);
        i++;
    }

    *bytesRead =  numSectors * 512;
    return OK;
}

enum FS_ERROR fat12_write_file(char path[], void** buf, uint32_t bytesToWrite)
{
    // Check if the filesystem has been initialized
    if(fs_info == NULL)
        return FS_UNINITIALIZED;

    // Identify the path and make sure it is valid
    Array path_split;
    strsplit_indices(path, '/', &path_split);
    if(path_split.used != 1 || path[0] != '/' || strlen(path) <= 1)
    {
        log(3, "Invalid path provided for FAT 12 filesystem.");
        return INVALID_PATH;
    }
    uint32_t nameLocation = ((uint32_t*)path_split.array)[0];
    uintptr_t fileName = mm_allocate(strlen(path) - 1 - nameLocation);
    mm_copy((uintptr_t)(path + nameLocation + 1), fileName, strlen(path) - 1 - nameLocation);
    freeArray(&path_split);

    // Make sure the file name isn't already taken
    struct FILE existingFile;
    if(fat12_find_file(path, &existingFile) != FILE_NOT_FOUND)
        return FILE_EXISTS;

    // Check to make sure there's enough space
    if(bytesToWrite > fat12_get_free_space())
        return FS_INSUFFICIENT_SPACE;
    
    int fileNameLength = strlen((char*)fileName);
    if(fileNameLength > 260)
        return INVALID_PATH;

    // Ensure file name length is divisible by 13
    char* paddedFileName = (char*)mm_allocate((uintptr_t)(fileNameLength + (13 - fileNameLength % 13) + 1));
    mm_copy((uintptr_t)fileName, (uintptr_t)paddedFileName, fileNameLength + 1);
    for(uint32_t i = fileNameLength; i < (fileNameLength + (13 - fileNameLength % 13) + 1); i++)
    {
        paddedFileName[i] = '\0';
    }

    // Find next empty root directory entry table entry
    uint8_t numEntriesRequired = 2;
    if(fileNameLength > 13)
        numEntriesRequired += (fileNameLength - 13) / 13 + 1;
    char* foundEntry = NULL;
    uint8_t numSuccessiveEntries = 0;
    for(uint32_t i = 0; i < fs_info->mbr->rootEntryCount; i++)
    {
        char* entry = (char*)(fs_info->rootDir) + (i * 32);

        uint8_t empty = 1;
        for(uint32_t j = 0; j < 4; j += 1) {
            if(*(uint64_t*)(entry + j) != 0)
            {
                empty = 0;
                break;
            }
        }

        if(empty == 1)
        {
            if(foundEntry == NULL)
                foundEntry = entry;
            numSuccessiveEntries++;
            if (numSuccessiveEntries == numEntriesRequired)
                break;
        }
        else if(foundEntry != NULL)
            foundEntry = NULL;
    }

    // If no empty entry was found, return
    if(foundEntry == NULL || numSuccessiveEntries != numEntriesRequired)
        return FS_INSUFFICIENT_SPACE;
    
    uint8_t checksum = fat12_checksum((char*)fileName);

    // Write the entries
    uint32_t fat_size = fs_info->mbr->fatSize;
    uint32_t fat_offset = fs_info->mbr->reservedSectorCount;
    uint32_t num_fats = fs_info->mbr->numFats;

    uint32_t root_size = fs_info->mbr->rootEntryCount * 32 / fs_info->mbr->bytesPerSector;
    uint32_t root_offset = fat_offset + (num_fats * fat_size);

    char* fileNamePointer = (char*)fileName;
    for(uint8_t i = 1; i < numEntriesRequired; i++)
    {
        // Turn long file name into chunks
        char unicodeBuf1[10] = "";
        ascii_to_unicode(fileNamePointer, unicodeBuf1, 5);
        fileNamePointer += 5;

        char unicodeBuf2[12] = "";
        ascii_to_unicode(fileNamePointer, unicodeBuf2, 6);
        fileNamePointer += 6;

        char unicodeBuf3[4] = "";
        ascii_to_unicode(fileNamePointer, unicodeBuf3, 2);
        fileNamePointer += 2;

        // Create long name entry
        char* currentEntry = foundEntry + ((numEntriesRequired - i - 1) * 32);
        currentEntry[0] = i;
        if(i == numEntriesRequired - 1)
            currentEntry[0] |= 0x40;
        mm_copy((uintptr_t)unicodeBuf1, (uintptr_t)(currentEntry + 1), 10);
        currentEntry[11] = 0xF;
        currentEntry[12] = 0;
        currentEntry[13] = checksum;
        mm_copy((uintptr_t)unicodeBuf2, (uintptr_t)(currentEntry + 14), 12);
        currentEntry[26] = 0;
        currentEntry[27] = 0;
        mm_copy((uintptr_t)unicodeBuf3, (uintptr_t)(currentEntry + 28), 4);

        write_sectors_to_disk(root_offset, ((uintptr_t)currentEntry - fs_info->rootDir), 32, (uintptr_t)currentEntry);
    }

    // Write final entry
    char* finalEntry = foundEntry + ((numEntriesRequired - 1) * 32);
    mm_copy((uintptr_t)fileName, (uintptr_t)(finalEntry), 11);
    finalEntry[11] = 0;
    finalEntry[12] = 0;
    finalEntry[13] = 0;
    finalEntry[14] = 0;
    finalEntry[15] = 0;
    finalEntry[16] = 0;
    finalEntry[17] = 0;
    finalEntry[18] = 0;
    finalEntry[19] = 0;
    finalEntry[20] = 0;
    finalEntry[21] = 0;
    finalEntry[22] = 0;
    finalEntry[23] = 0;
    finalEntry[24] = 0;
    finalEntry[25] = 0;
    *(uint32_t*)&(finalEntry[28]) = bytesToWrite;

    // Mark FAT clusters
    uint16_t* usedClusters = (uint16_t*)mm_allocate(sizeof(uint16_t) * (bytesToWrite / fs_info->mbr->bytesPerSector + 1));

    uint32_t clusterCnt = 0;
    uint32_t clusterIdx = 2;
    while(clusterCnt < bytesToWrite / fs_info->mbr->bytesPerSector + 1)
    {
        uint16_t cluster = fat12_get_cluster(clusterIdx);

        if(cluster == 0x0)
        {
            usedClusters[clusterCnt] = clusterIdx;
            if(clusterCnt > 0)
                fat12_set_cluster(usedClusters[clusterCnt - 1], clusterIdx);
            clusterCnt++;
        }
        clusterIdx++;
    }
    fat12_set_cluster(usedClusters[clusterCnt - 1], 0xFFF);

    // Set first FAT cluster in directory entry
    *(uint16_t*)&(finalEntry[26]) = usedClusters[0];

    write_sectors_to_disk(root_offset, ((uintptr_t)finalEntry - fs_info->rootDir), 32, (uintptr_t)finalEntry);

    // Write file contents to disk
    uint32_t data_offset = fs_info->mbr->reservedSectorCount + (fs_info->mbr->numFats * fs_info->mbr->fatSize) + (fs_info->mbr->rootEntryCount * 32 / fs_info->mbr->bytesPerSector);

    char* buffer = *buf;

    for(int i = 0; i < clusterCnt; i++)
    {
        uint16_t writeLength;
        if(i == clusterCnt - 1)
            writeLength = bytesToWrite % fs_info->mbr->bytesPerSector;
        else
            writeLength = fs_info->mbr->bytesPerSector;

        uint32_t addr = data_offset + (usedClusters[i] - 2) * fs_info->mbr->sectorsPerCluster;

        write_sectors_to_disk(addr, 0, writeLength, (uintptr_t)buffer);

        buffer += fs_info->mbr->bytesPerSector;

        if(i == clusterCnt - 1)
        {
            uintptr_t zeros = mm_allocate(fs_info->mbr->bytesPerSector - writeLength);
            mm_set(zeros, 0, fs_info->mbr->bytesPerSector - writeLength);
            write_sectors_to_disk(addr, writeLength, fs_info->mbr->bytesPerSector - writeLength, zeros);
            mm_free(zeros);
        }
    }

    fs_info->bytesFree -= clusterCnt * fs_info->mbr->bytesPerSector * fs_info->mbr->sectorsPerCluster;

    mm_free((uintptr_t)usedClusters);

    return OK;
}

uint8_t fat12_checksum(char* shortName)
{
    int16_t nameLength;
    uint8_t sum;
    for(nameLength = 11; nameLength != 0; nameLength--)
    {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *shortName++;
    }
    return sum;
}