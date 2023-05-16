#ifndef KERNEL_H
#define KERNEL_H

#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../cpu/isr.h"
#include "../libc/mem.h"

void user_input(char *input);

#endif
