#ifndef KERNEL_PCI
#define KERNEL_PCI

#include "../cpu/ports.h"
#include "../kernel/log.h"
#include "../kernel_lib/string.h"
#include "../kernel_lib/dynamic_array.h"

struct PCI_DEVICE
{
    uint8_t baseClass;
    uint8_t subClass;
    uint8_t progIF;
    uint16_t deviceID;
    uint16_t vendorID;
};

Array* prepare_kernel_pci();
Array* pci_get_devices();
void pci_log_device(struct PCI_DEVICE* dev);

void pciEnumerate(Array* pci_devices);
void pciCheckBus(uint8_t bus, Array* pci_devices);
void pciCheckDevice(uint8_t bus, uint8_t device, Array* pci_devices);
void pciCheckFunction(uint8_t bus, uint8_t device, uint8_t function, Array* pci_devices);

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pciGetVendorID(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pciGetDeviceID(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetHeaderType(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetBaseClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetSubClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetSecondaryBus(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pciGetProgIF(uint8_t bus, uint8_t slot, uint8_t function);

#endif
