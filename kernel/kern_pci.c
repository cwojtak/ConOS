#include "kern_pci.h"

Array* prepare_kernel_pci()
{
    Array* pci_devices = NULL;
    initArray(pci_devices, 16);
    pciEnumerate(pci_devices);
    return pci_devices;
}

void pciEnumerate(Array* pci_devices)
{
    uint8_t function;
    uint8_t bus;
    
    uint8_t headerType = pciGetHeaderType(0, 0, 0);
    if ((headerType & 0x80) == 0)
    {
        pciCheckBus(0, pci_devices); //Single PCI host controller
    }
    else
    {
        for (function = 0; function < 8; function++)
        {
            if (pciGetVendorID(0, 0, function) != 0xFFFF) break;
            bus = function;
            pciCheckBus(bus, pci_devices);
        }
    }
}

void pciCheckBus(uint8_t bus, Array* pci_devices)
{
    for (uint8_t device = 0; device < 32; device++)
        pciCheckDevice(bus, device, pci_devices);
}

void pciCheckDevice(uint8_t bus, uint8_t device, Array* pci_devices)
{
    uint8_t function = 0;
    
    uint16_t vendorID = pciGetVendorID(bus, device, function);
    if (vendorID == 0xFFFF) return; //All ones indicates device does not exist
    pciCheckFunction(bus, device, function, pci_devices);
    uint8_t headerType = pciGetHeaderType(bus, device, function);
    if ((headerType & 0x80 != 0 )) //Check for multiple functions
    {
        for (function = 1; function < 8; function++)
        {
            if (pciGetVendorID(bus, device, function) != 0xFFFF)
            {
                pciCheckFunction(bus, device, function, pci_devices);
            }
        }
    }
}

void pciCheckFunction(uint8_t bus, uint8_t device, uint8_t function, Array* pci_devices)
{
    uint8_t baseClass = pciGetBaseClass(bus, device, function);
    uint8_t subClass = pciGetSubClass(bus, device, function);
    uint8_t progIF = pciGetProgIF(bus, device, function);

    struct PCI_DEVICE* dev = (struct PCI_DEVICE*)mm_allocate(sizeof(struct PCI_DEVICE));
    dev->baseClass = baseClass;
    dev->subClass = subClass;
    dev->progIF = progIF;
    dev->deviceID = pciGetDeviceID(bus, device, function);
    dev->vendorID = pciGetVendorID(bus, device, function);

    insertArray(pci_devices, (uintptr_t)dev);

    //Detect secondary busses
    if((baseClass == 0x6) && (subClass == 0x4))
    {
        pciCheckBus(pciGetSecondaryBus(bus, device, function), pci_devices);
    }
    //Detect IDE controllers for disk access
    else if(baseClass == 0x1 && subClass == 0x1)
    {
        //Check if controller is in compatibility mode
        if(progIF & 0x1 == 0)
        {
            
        }
    }
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    port_dword_out(0xCF8, address);

    return (uint16_t)((port_dword_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint16_t pciGetVendorID(uint8_t bus, uint8_t slot, uint8_t function)
{
    return pciConfigReadWord(bus, slot, function, 0);
}

uint16_t pciGetDeviceID(uint8_t bus, uint8_t slot, uint8_t function)
{
    return pciConfigReadWord(bus, slot, function, 0x2);
}

uint8_t pciGetHeaderType(uint8_t bus, uint8_t slot, uint8_t function)
{
    return (uint8_t)(pciConfigReadWord(bus, slot, function, 0x0E) & 0xFF);
}

uint8_t pciGetBaseClass(uint8_t bus, uint8_t slot, uint8_t function)
{
    return (uint8_t)(pciConfigReadWord(bus, slot, function, 0x0B) & 0xFF);
}

uint8_t pciGetSubClass(uint8_t bus, uint8_t slot, uint8_t function)
{
    return (uint8_t)(pciConfigReadWord(bus, slot, function, 0x0A) & 0xFF);
}

uint8_t pciGetSecondaryBus(uint8_t bus, uint8_t slot, uint8_t function)
{
    return (uint8_t)(pciConfigReadWord(bus, slot, function, 0x19) & 0xFF);
}

uint8_t pciGetProgIF(uint8_t bus, uint8_t slot, uint8_t function)
{
    return (uint8_t)(pciConfigReadWord(bus, slot, function, 0x9) & 0xFF);
}
