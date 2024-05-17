#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "ports.h"
#include "isr.h"
#include "../libc/function.h"

void init_timer(uint32_t freq);

#endif
