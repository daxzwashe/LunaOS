#include <stdint.h>
#include "../include/memory.h"

uint8_t* Bitmap = 0;
uint64_t BitmapSize = 0;
uint64_t TotalMemory = 0;

void Bitmap_Set(uint64_t bit) { Bitmap[bit / 8] |= (1 << (bit % 8)); }
void Bitmap_Unset(uint64_t bit) { Bitmap[bit / 8] &= ~(1 << (bit % 8)); }
int Bitmap_Get(uint64_t bit) { return (Bitmap[bit / 8] >> (bit % 8)) & 1; }

void PMM_Init(MemoryInfo* memInfo) {
    TotalMemory = 0;
    uint64_t entries = memInfo->mMapSize / memInfo->mMapDescSize;

    for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memInfo->mMap + (i * memInfo->mMapDescSize));
        TotalMemory += desc->NumberOfPages * 4096;
    }

    uint64_t totalPages = TotalMemory / 4096;
    BitmapSize = totalPages / 8 + 1;

    // Ищем место для битмапа
    for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memInfo->mMap + (i * memInfo->mMapDescSize));
        if (desc->Type == 7 && (desc->NumberOfPages * 4096) > BitmapSize) {
            Bitmap = (uint8_t*)desc->PhysicalStart;
            break;
        }
    }

    // Инициализация битмапа
    for(uint64_t i=0; i<BitmapSize; i++) Bitmap[i] = 0xFF; // Все занято

    for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memInfo->mMap + (i * memInfo->mMapDescSize));
        if (desc->Type == 7) { // Free Memory
            uint64_t start = desc->PhysicalStart / 4096;
            for (uint64_t j = 0; j < desc->NumberOfPages; j++) Bitmap_Unset(start + j);
        }
    }

    // Блокируем сам битмап и первый мегабайт
    uint64_t bitmapPage = (uint64_t)Bitmap / 4096;
    for(uint64_t i=0; i < (BitmapSize/4096 + 1); i++) Bitmap_Set(bitmapPage + i);
    for(int i=0; i<256; i++) Bitmap_Set(i);
}

void* PMM_AllocPage() {
    uint64_t totalPages = TotalMemory / 4096;
    for (uint64_t i = 0; i < totalPages; i++) {
        if (!Bitmap_Get(i)) {
            Bitmap_Set(i);
            return (void*)(i * 4096);
        }
    }
    return 0;
}

void PMM_FreePage(void* address) {
    uint64_t page = (uint64_t)address / 4096;
    Bitmap_Unset(page);
}

uint64_t PMM_GetFreeMemory() {
    uint64_t free = 0;
    uint64_t totalPages = TotalMemory / 4096;
    for (uint64_t i=0; i<totalPages; i++) {
        if (!Bitmap_Get(i)) free++;
    }
    return free * 4096;
}