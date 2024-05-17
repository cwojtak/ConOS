#ifndef COM_H
#define COM_H

#include "../cpu/ports.h"
#include "screen.h"

int init_serial();
int serial_received();
char read_serial();
int is_transmit_empty();
void write_serial(char c);
void write_string_serial(char* c);
void read_string_serial(char* c);

#endif
