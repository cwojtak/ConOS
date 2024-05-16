#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../cpu/type.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../libc/function.h"
#include "../kernel/kernel.h"

void init_keyboard();

#endif
