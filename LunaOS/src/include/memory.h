#pragma once
#include <stdint.h>
#include "framebuffer.h"

// ... (старые структуры EFI оставляем) ...
typedef struct {
    uint32_t Type; uint32_t Pad; uint64_t PhysicalStart; uint64_t VirtualStart; uint64_t NumberOfPages; uint64_t Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct {
    EFI_MEMORY_DESCRIPTOR* mMap;
    uint64_t mMapSize;
    uint64_t mMapDescSize;
} MemoryInfo;

typedef struct {
    Framebuffer* fb;
    const void* font;
    MemoryInfo* memInfo;
} BootInfo;

// PMM (Physical Memory Manager)
void PMM_Init(MemoryInfo* memInfo);
void* PMM_AllocPage();
void PMM_FreePage(void* address);
uint64_t PMM_GetFreeMemory();

// HEAP (Dynamic Memory Manager) - НОВОЕ
void Heap_Init(void* startAddress, size_t sizeBytes);
void* malloc(size_t size);
void free(void* ptr);