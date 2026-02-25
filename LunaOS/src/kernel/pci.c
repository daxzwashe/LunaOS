#include <stdint.h>
#include "../include/io.h"
#include "../include/pci.h"

// Внешние функции печати (из kernel.c)
extern void kprintHex(uint64_t num);
extern void kprint(char* str);
extern void kprintInt(int num);

// Глобальное хранилище для найденного диска
PCIDevice AHCIDevice;
int AHCI_Found = 0;

// Чтение 32 бит из конфигурации PCI
uint32_t PCI_Read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
 
    // Создаем адрес запроса
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    
    // Отправляем запрос в порт 0xCF8
    outl(0xCF8, address);
    
    // Читаем ответ из порта 0xCFC
    return inl(0xCFC);
}

// Чтение 16 бит (для Vendor ID и Device ID)
uint16_t PCI_Read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t res = PCI_Read32(bus, slot, func, offset);
    return (uint16_t)((res >> ((offset & 2) * 8)) & 0xFFFF);
}

// Проверка одного устройства
void CheckDevice(uint8_t bus, uint8_t device) {
    uint8_t function = 0;
 
    uint16_t vendorID = PCI_Read16(bus, device, function, 0);
    if (vendorID == 0xFFFF) return; // Устройства нет
 
    uint16_t deviceID = PCI_Read16(bus, device, function, 2);
    uint16_t classWord = PCI_Read16(bus, device, function, 0x0A);
    uint8_t classCode = (classWord >> 8) & 0xFF;
    uint8_t subclass = classWord & 0xFF;

    // --- ВЫВОД НАЙДЕННОГО УСТРОЙСТВА ---
    /* 
       Раскомментируй строки ниже, если хочешь видеть список ВСЕХ устройств.
       Пока выводим только SATA, чтобы не засорять экран.
    */
    /*
    kprint("[PCI] "); 
    kprintHex(vendorID); kprint(":"); kprintHex(deviceID);
    kprint(" Class:"); kprintHex(classCode);
    kprint(" Sub:"); kprintHex(subclass);
    kprint("\n");
    */

    // Ищем SATA Контроллер (Class 0x01, Subclass 0x06)
    if (classCode == 0x01 && subclass == 0x06) {
        kprint(">>> FOUND SATA AHCI! <<<\n");
        
        AHCIDevice.VendorID = vendorID;
        AHCIDevice.DeviceID = deviceID;
        AHCIDevice.ClassID = classCode;
        AHCIDevice.SubclassID = subclass;
        AHCIDevice.Bus = bus;
        AHCIDevice.Device = device;
        AHCIDevice.Function = function;
        
        // Читаем адрес управления (BAR5)
        AHCIDevice.PortBase = PCI_Read32(bus, device, function, 0x24); 
        
        AHCI_Found = 1;

        kprint("    BAR5 Address: ");
        kprintHex(AHCIDevice.PortBase);
        kprint("\n");
    }
}

// Сканирование всей шины
void PCI_Scan() {
    AHCI_Found = 0;
    kprint("Scanning PCI Bus...\n");
    
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            CheckDevice((uint8_t)bus, device);
        }
    }
    
    if (AHCI_Found) {
        kprint("[ OK ] AHCI Found. Ready for driver.\n");
    } else {
        kprint("[FAIL] No SATA/AHCI found. Check QEMU flags.\n");
    }
}