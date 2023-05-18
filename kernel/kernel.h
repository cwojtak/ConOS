#ifndef KERNEL_H
#define KERNEL_H

#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../cpu/isr.h"
#include "../libc/mem.h"

struct multiboot_info {
    uint32_t flags;
    uint32_t memoryLo;
    uint32_t memoryHi;
    uint32_t bootDevice;
    uint32_t cmdLine;
    uint32_t modsCount;
    uint32_t modsAddr;
    uint32_t sysm0;
    uint32_t sysm1;
    uint32_t sysm2;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t bootloader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint32_t vbe_interface_addr;
    uint16_t vbe_interface_len;
};

void user_input(char* input);

#endif
