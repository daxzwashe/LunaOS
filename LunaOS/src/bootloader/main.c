#include <stdint.h>
#include "../include/framebuffer.h"
#include "../include/memory.h" // Структура EFI_MEMORY_DESCRIPTOR теперь здесь

// === СТАНДАРТНЫЕ ФУНКЦИИ ===
void* memcpy(void* dest, const void* src, uint64_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (uint64_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

void* memset(void* s, int c, uint64_t n) {
    uint8_t* p = (uint8_t*)s;
    for (uint64_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return s;
}

// === СТРУКТУРЫ UEFI ===
typedef void* EFI_HANDLE;
typedef uint64_t EFI_STATUS;
#define EFI_SUCCESS 0

typedef struct { uint32_t Data1; uint16_t Data2; uint16_t Data3; uint8_t Data4[8]; } EFI_GUID;
EFI_GUID EFI_GOP_GUID = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

typedef struct { uint64_t Signature; uint32_t Revision; uint32_t HeaderSize; uint32_t CRC32; uint32_t Reserved; } EFI_TABLE_HEADER;

// ВНИМАНИЕ: EFI_MEMORY_DESCRIPTOR удален отсюда, так как он есть в memory.h

typedef struct {
    uint32_t Version; uint32_t HorizontalResolution; uint32_t VerticalResolution; int PixelFormat; uint32_t PixelInformation[4]; uint32_t PixelsPerScanLine;
} EFI_GOP_MODE_INFO;

typedef struct {
    uint32_t MaxMode; uint32_t Mode; EFI_GOP_MODE_INFO* Info; uint64_t SizeOfInfo; uint64_t FrameBufferBase; uint64_t FrameBufferSize;
} EFI_GOP_MODE;

typedef struct _EFI_GOP { void* QueryMode; void* SetMode; void* Blt; EFI_GOP_MODE* Mode; } EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr; void* RaiseTPL; void* RestoreTPL; void* AllocatePages; void* FreePages;
    // Используем EFI_MEMORY_DESCRIPTOR из memory.h
    EFI_STATUS (*GetMemoryMap)(uint64_t* MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, uint64_t* MapKey, uint64_t* DescriptorSize, uint32_t* DescriptorVersion);
    void* AllocatePool; void* FreePool; void* CreateEvent; void* SetTimer; void* WaitForEvent; void* SignalEvent; void* CloseEvent; void* CheckEvent; void* InstallProtocolInterface; void* ReinstallProtocolInterface; void* UninstallProtocolInterface; void* HandleProtocol; void* Reserved; void* RegisterProtocolNotify; void* LocateHandle; void* LocateDevicePath; void* InstallConfigurationTable; void* LoadImage; void* StartImage; void* Exit; void* UnloadImage;
    EFI_STATUS (*ExitBootServices)(EFI_HANDLE ImageHandle, uint64_t MapKey);
    void* GetNextMonotonicCount; void* Stall; void* SetWatchdogTimer; void* ConnectController; void* DisconnectController; void* OpenProtocol; void* CloseProtocol; void* OpenProtocolInformation; void* ProtocolsPerHandle; void* LocateHandleBuffer;
    EFI_STATUS (*LocateProtocol)(EFI_GUID* Protocol, void* Registration, void** Interface);
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr; uint16_t* FirmwareVendor; uint32_t FirmwareRevision; void* ConsoleInHandle; void* ConsoleIn; void* ConsoleOutHandle; void* ConsoleOut; void* StandardErrorHandle; void* StandardError; void* RuntimeServices; EFI_BOOT_SERVICES* BootServices; uint64_t NumberOfTableEntries; void* ConfigurationTable;
} EFI_SYSTEM_TABLE;

// Буферы для передачи ядру
uint8_t MemoryMap[1024 * 16];
MemoryInfo memInfo;
BootInfo bootInfo;

// Объявляем внешнюю функцию ядра
void KernelStart(BootInfo* bootInfo);

// === MAIN ===
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    // 1. Ищем GOP (Графику)
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;
    SystemTable->BootServices->LocateProtocol(&EFI_GOP_GUID, 0, (void**)&Gop);

    // 2. Готовим структуру графики для ядра
    Framebuffer fb;
    fb.BaseAddress = (void*)Gop->Mode->FrameBufferBase;
    fb.BufferSize = Gop->Mode->FrameBufferSize;
    fb.Width = Gop->Mode->Info->HorizontalResolution;
    fb.Height = Gop->Mode->Info->VerticalResolution;
    fb.PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;

    // 3. Получаем карту памяти
    uint64_t MapSize = sizeof(MemoryMap);
    uint64_t MapKey;
    uint64_t DescriptorSize;
    uint32_t DescriptorVersion;
    
    SystemTable->BootServices->GetMemoryMap(&MapSize, (EFI_MEMORY_DESCRIPTOR*)MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    // Заполняем инфо о памяти для ядра
    memInfo.mMap = (EFI_MEMORY_DESCRIPTOR*)MemoryMap;
    memInfo.mMapSize = MapSize;
    memInfo.mMapDescSize = DescriptorSize;

    // Собираем BootInfo
    bootInfo.fb = &fb;
    bootInfo.memInfo = &memInfo;

    // 4. Выходим из UEFI
    SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

    // 5. ПЕРЕДАЕМ УПРАВЛЕНИЕ ЯДРУ
    KernelStart(&bootInfo);

    return EFI_SUCCESS;
}