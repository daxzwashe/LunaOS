#pragma once
#include <stdint.h>

typedef struct {
    uint32_t PortBase;
    uint32_t Interrupt;
    uint16_t Bus;
    uint16_t Device;
    uint16_t Function;
    uint16_t VendorID;
    uint16_t DeviceID;
    uint8_t ClassID;
    uint8_t SubclassID;
    uint8_t ProgIF; // Интерфейс программирования (нужен чтобы отличить AHCI от IDE)
} PCIDevice;

// Глобальная переменная для хранения найденного контроллера AHCI
extern PCIDevice AHCIDevice;
extern int AHCI_Found;

void PCI_Scan();