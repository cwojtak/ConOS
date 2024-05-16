#include "ports.h"

/**
  * Read a byte from the specified port/address.
  */
uint8_t port_byte_in(uint16_t port) {
	uint8_t result;
	asm("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

/**
  * Write a byte to the specified port/address.
  */
void port_byte_out(uint16_t port, uint8_t data) {
	asm("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t port_word_in(uint16_t port) {
	uint16_t result;
	asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void port_word_out(uint16_t port, uint16_t data) {
	asm("out %%ax, %%dx" : : "a" (data), "d" (port));
}

uint32_t port_dword_in(uint32_t port)
{
	uint32_t result;
	asm("inl %%edx, %%eax" : "=a" (result) : "d" (port));
	return result;
}

void port_dword_out(uint32_t port, uint32_t data)
{
    asm("out %%eax, %%edx" : : "a" (data), "d" (port));
}
