#ifndef KERNEL_PCI
#define KERNEL_PCI

#include "../cpu/ports.h"
#include "../kernel/log.h"

void prepare_kernel_pci();

void pciEnumerate();
void pciCheckBus(uint8_t bus);
void pciCheckDevice(uint8_t bus, uint8_t device);
void pciCheckFunction(uint8_t bus, uint8_t device, uint8_t function);

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pciGetVendorID(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetHeaderType(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetBaseClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetSubClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetSecondaryBus(uint8_t bus, uint8_t slot, uint8_t function);

#endif
