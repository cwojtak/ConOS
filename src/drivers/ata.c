#include "ata.h"

int ata_try_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf);
int ata_try_write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf);


int ata_try_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf)
{
    if(lba_sector > 0x0FFFFFFF) return -1;

    //Send command to read the disk
    port_byte_out(0x01F6, (uint8_t)((lba_sector >> 24) | (224)));
    port_byte_out(0x01F2, num_sectors);
    port_byte_out(0x01F3, (uint8_t)(lba_sector & 0xFF));
    port_byte_out(0x01F4, (uint8_t)((lba_sector >> 8) & 0xFF));
    port_byte_out(0x01F5, (uint8_t)((lba_sector >> 16) & 0xFF));
    port_byte_out(0x01F7, 0x20);

    uint8_t test_op;

    // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
    while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);

    // Check if ERR (bit 0) is set which indicates an error
    if (test_op & 0x01) return -1; 

    // Continue with reading data from the disk
    for(int i = 0; i < 256 * num_sectors; i++)
    {
        if(i % 256 == 0 && i != 0)
        {
            // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
            while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);
        }
        uint16_t data = port_word_in(0x01F0);
        *(uint16_t *)(buf + i * 2) = data;
    }

    return 0;
}

int ata_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf)
{
    for (int attempt = 0; attempt < 3; attempt++) 
    {
        int result = ata_try_read_sectors_from_disk(lba_sector, num_sectors, buf);
        if (result == 0)
            return result;
    }
    log(3, "ATA disk read failed with retries.");
    return -1;
} 

int ata_try_write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf)
{
    if(lba_sector > 0x0FFFFFFF || offset > 511) return -1;

    uint32_t numSectors = (length / 512) + (((length % 512) + offset) / 512) + 1;

    //Read the original contents of the first and last sectors
    uint8_t firstSectorOrgContent[512];
    uint8_t lastSectorOrgContent[512];
    if(ata_try_read_sectors_from_disk(lba_sector, 1, (uintptr_t)firstSectorOrgContent) != 0 ||
        ata_try_read_sectors_from_disk(lba_sector + numSectors - 1, 1, (uintptr_t)lastSectorOrgContent) != 0)
            return -1;

    //Write the content to the first sector buffer
    uint8_t* dataBuf = (uint8_t*)buf;
    uint16_t bufOffset = 0;
    uint16_t i = offset;
    for(; i < 512 && bufOffset < length; i++)
    {
        firstSectorOrgContent[i] = dataBuf[bufOffset];
        bufOffset++;
    }
    //Write the content to the last sector buffer
    if(bufOffset < length && numSectors > 1)
    {
        uint16_t finalSectorBufOffset = bufOffset + ((numSectors - 2) * 512);
        for(i = 0; i < 512 && finalSectorBufOffset < length; i++)
        {
            lastSectorOrgContent[i] = dataBuf[finalSectorBufOffset];
            finalSectorBufOffset++;
        }
    }

    //Send command to write the first sector to the disk
    port_byte_out(0x01F6, (uint8_t)((lba_sector >> 24) | (224)));
    port_byte_out(0x01F2, 1);
    port_byte_out(0x01F3, (uint8_t)(lba_sector & 0xFF));
    port_byte_out(0x01F4, (uint8_t)((lba_sector >> 8) & 0xFF));
    port_byte_out(0x01F5, (uint8_t)((lba_sector >> 16) & 0xFF));
    port_byte_out(0x01F7, 0x30);

    uint8_t test_op;

    // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
    while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);

    // Check if ERR (bit 0) is set which indicates an error
    if (test_op & 0x01) return -1; 

    // Write first sector to the disk
    for(int i = 0; i < 256; i++)
    {
        if(i % 256 == 0 && i != 0)
        {
            // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
            while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);
        }
        port_word_out(0x01F0, *(uint16_t*)(firstSectorOrgContent + i * 2));
    }

    //Continue writing additional sectors
    if(numSectors > 2)
    {
        uint32_t lba_sector_next = lba_sector + 1;

        //Send command to write the disk
        port_byte_out(0x01F6, (uint8_t)((lba_sector_next >> 24) | (224)));
        port_byte_out(0x01F2, numSectors - 2);
        port_byte_out(0x01F3, (uint8_t)(lba_sector_next & 0xFF));
        port_byte_out(0x01F4, (uint8_t)((lba_sector_next >> 8) & 0xFF));
        port_byte_out(0x01F5, (uint8_t)((lba_sector_next >> 16) & 0xFF));
        port_byte_out(0x01F7, 0x30);

        // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
        while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);

        // Check if ERR (bit 0) is set which indicates an error
        if (test_op & 0x01) return -1; 

        // Write the sectors to the disk
        for(int i = 0; i < 256 * (numSectors - 2); i++)
        {
            if(i % 256 == 0 && i != 0)
            {
                // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
                while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);
            }
            port_word_out(0x01F0, *(uint16_t*)(buf + bufOffset));
            bufOffset += 2;
        }
    }
    if(numSectors > 1)
    {
        uint32_t lba_sector_final = lba_sector + numSectors - 1;

        //Send command to write the disk
        port_byte_out(0x01F6, (uint8_t)((lba_sector_final >> 24) | (224)));
        port_byte_out(0x01F2, 1);
        port_byte_out(0x01F3, (uint8_t)(lba_sector_final & 0xFF));
        port_byte_out(0x01F4, (uint8_t)((lba_sector_final >> 8) & 0xFF));
        port_byte_out(0x01F5, (uint8_t)((lba_sector_final >> 16) & 0xFF));
        port_byte_out(0x01F7, 0x30);

        // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
        while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);

        // Check if ERR (bit 0) is set which indicates an error
        if (test_op & 0x01) return -1; 

        // Write the sectors to the disk
        for(int i = 0; i < 256; i++)
        {
            if(i % 256 == 0 && i != 0)
            {
                // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
                while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);
            }
            port_word_out(0x01F0, *(uint16_t*)(lastSectorOrgContent + i * 2));
        }
    }

    // Wait until BSY (bit 7) is 0 (aka Wait for disk read to complete)
    while (((test_op = port_byte_in(0x1F7)) & 0x80) != 0);

    return 0;
}

int ata_write_sectors_to_disk(uint32_t lba_sector, uint16_t offset, uint16_t length, uintptr_t buf)
{
    for (int attempt = 0; attempt < 3; attempt++) 
    {
        int result = ata_try_write_sectors_to_disk(lba_sector, offset, length, buf);
        if (result == 0)
            return result;
    }
    log(3, "ATA disk write failed with retries.");
    return -1;
}
