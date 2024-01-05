#include "ata.h"

int ata_read_sectors_from_disk(uint32_t lba_sector, uint8_t num_sectors, uintptr_t buf)
{
    if(lba_sector > 0x0FFFFFFF) return -1;

    //Send command to read the disk
    port_byte_out(0x01F6, (uint8_t)((lba_sector >> 24) | (224)));
    port_byte_out(0x01F2, num_sectors);
    port_byte_out(0x01F3, (uint8_t)(lba_sector & 0xFF));
    port_byte_out(0x01F4, (uint8_t)((lba_sector >> 8) & 0xFF));
    port_byte_out(0x01F5, (uint8_t)((lba_sector >> 16) & 0xFF));
    port_byte_out(0x01F7, 0x20);

    //Wait for disk read to complete
    uint8_t test_op = 0;
    while(test_op && 8 != 0)
    {
        test_op = port_byte_in(0x1F7);
    }

    for(int i = 0; i < 256 * num_sectors; i++)
	{
		uint16_t data = port_word_in(0x01F0);
		*(uint16_t *)(buf + i * 2) = data;
	}

    return 0;
} 
