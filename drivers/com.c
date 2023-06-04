#include "com.h"

int init_serial() {
    const int PORT = 0x3f8;
    port_byte_out(PORT + 1, 0x00);
    port_byte_out(PORT + 3, 0x80);
    port_byte_out(PORT + 0, 0x03);
    port_byte_out(PORT + 1, 0x00);
    port_byte_out(PORT + 3, 0x03);
    port_byte_out(PORT + 2, 0xC7);
    port_byte_out(PORT + 4, 0x0B);
    port_byte_out(PORT + 4, 0x1E);
    port_byte_out(PORT + 0, 0xAE);

    if(port_byte_in(PORT + 0) != 0xAE) {
        kprint("Warning: Failed to initialize COM1.\n");
        return 1;
    }

    port_byte_out(PORT + 4, 0x0F);
    return 0;
}

int serial_received() {
    return port_byte_in(0x3fd) & 1;
}

char read_serial() {
    while(serial_received() == 0);

    return port_byte_in(0x3f8);
}

int is_transmit_empty() {
    return port_byte_in(0x3fd) & 0x20;
}

void write_serial(char c) {
    while(is_transmit_empty() == 0);

    port_byte_out(0x3f8, c);
}

void write_string_serial(char* c) {
    for(uint32_t i = 0; c[i] != '\0'; i++) {
        write_serial(c[i]);
    }
    write_serial('\0');
}

void read_string_serial(char* c) {
    for(uint32_t i = 0; c[i] != '\0'; i++) {
        c[i] = read_serial();
    }
    write_serial('\0');
}
