#include "kern_pci.h"

void prepare_kernel_pci()
{
    pciEnumerate();
}

void pciEnumerate()
{
    uint8_t function;
    uint8_t bus;
    
    uint8_t headerType = pciGetHeaderType(0, 0, 0);
    if ((headerType & 0x80) == 0)
    {
        pciCheckBus(0); //Single PCI host controller
    }
    else
    {
        for (function = 0; function < 8; function++)
        {
            if (pciGetVendorID(0, 0, function) != 0xFFFF) break;
            bus = function;
            pciCheckBus(bus);
        }
    }
}

void pciCheckBus(uint8_t bus)
{
    for (uint8_t device = 0; device < 32; device++)
        pciCheckDevice(bus, device);
}

void pciCheckDevice(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;
    
    uint16_t vendorID = pciGetVendorID(bus, device, function);
    if (vendorID == 0xFFFF) return; //All ones indicates device does not exist
    pciCheckFunction(bus, device, function);
    
    uint8_t headerType = pciGetHeaderType(bus, device, function);
    if ((headerType & 0x80 != 0 )) //Check for multiple functions
    {
        for (function = 1; function < 8; function++)
        {
            if (pciGetVendorID(bus, device, function) != 0xFFFF)
            {
                pciCheckFunction(bus, device, function);
            }
        }
    }
}

void pciCheckFunction(uint8_t bus, uint8_t device, uint8_t function)
{
    uint8_t baseClass = pciGetBaseClass(bus, device, function);
    uint8_t subClass = pciGetSubClass(bus, device, function);

    //Detect secondary busses
    if((baseClass == 0x6) && (subClass == 0x4))
    {
        pciCheckBus(pciGetSecondaryBus(bus, device, function));
    }
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | ((offset & 0xFC) | ((uint32_t)0x80000000)));

    port_byte_out(0xCF8, address);
    
    return (uint16_t)((port_byte_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint16_t pciGetVendorID(uint8_t bus, uint8_t slot, uint8_t function)
{
    return pciConfigReadWord(bus, slot, function, 0);
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
