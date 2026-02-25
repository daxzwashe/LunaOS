#include <stdint.h>
#include <stddef.h>
#include "../include/memory.h"

// Заголовок блока памяти
typedef struct MemorySegmentHeader {
    uint64_t MemoryLength;
    struct MemorySegmentHeader* NextSegment;
    struct MemorySegmentHeader* PreviousSegment;
    struct MemorySegmentHeader* NextFreeSegment;
    struct MemorySegmentHeader* PreviousFreeSegment;
    int Free;
} MemorySegmentHeader;

MemorySegmentHeader* FirstFreeMemorySegment;

void Heap_Init(void* startAddress, size_t sizeBytes) {
    MemorySegmentHeader* currentSegment = (MemorySegmentHeader*)startAddress;
    currentSegment->MemoryLength = sizeBytes - sizeof(MemorySegmentHeader);
    currentSegment->NextSegment = NULL;
    currentSegment->PreviousSegment = NULL;
    currentSegment->NextFreeSegment = NULL;
    currentSegment->PreviousFreeSegment = NULL;
    currentSegment->Free = 1;

    FirstFreeMemorySegment = currentSegment;
}

void* malloc(size_t size) {
    uint64_t remainder = size % 8;
    if (remainder != 0) size += 8 - remainder;

    MemorySegmentHeader* currentMemorySegment = FirstFreeMemorySegment;

    while (currentMemorySegment != NULL) {
        if (currentMemorySegment->MemoryLength >= size) {
            if (currentMemorySegment->MemoryLength > size + sizeof(MemorySegmentHeader)) {
                MemorySegmentHeader* newSegmentHeader = (MemorySegmentHeader*)((uint64_t)currentMemorySegment + sizeof(MemorySegmentHeader) + size);
                newSegmentHeader->Free = 1;
                newSegmentHeader->MemoryLength = ((uint64_t)currentMemorySegment->MemoryLength) - (sizeof(MemorySegmentHeader) + size);
                newSegmentHeader->NextFreeSegment = currentMemorySegment->NextFreeSegment;
                newSegmentHeader->PreviousFreeSegment = currentMemorySegment->PreviousFreeSegment;
                newSegmentHeader->NextSegment = currentMemorySegment->NextSegment;
                newSegmentHeader->PreviousSegment = currentMemorySegment;
                currentMemorySegment->NextFreeSegment = newSegmentHeader;
                currentMemorySegment->NextSegment = newSegmentHeader;
                currentMemorySegment->MemoryLength = size;
            }
            if (currentMemorySegment == FirstFreeMemorySegment) {
                FirstFreeMemorySegment = currentMemorySegment->NextFreeSegment;
            }
            currentMemorySegment->Free = 0;
            if (currentMemorySegment->PreviousFreeSegment != NULL) currentMemorySegment->PreviousFreeSegment->NextFreeSegment = currentMemorySegment->NextFreeSegment;
            if (currentMemorySegment->NextFreeSegment != NULL) currentMemorySegment->NextFreeSegment->PreviousFreeSegment = currentMemorySegment->PreviousFreeSegment;

            return (void*)((uint64_t)currentMemorySegment + sizeof(MemorySegmentHeader));
        }
        currentMemorySegment = currentMemorySegment->NextFreeSegment;
    }
    return NULL;
}

void free(void* address) {
    MemorySegmentHeader* currentMemorySegment = (MemorySegmentHeader*)((uint64_t)address - sizeof(MemorySegmentHeader));
    currentMemorySegment->Free = 1;
    if (currentMemorySegment < FirstFreeMemorySegment) FirstFreeMemorySegment = currentMemorySegment;
    if (currentMemorySegment->NextFreeSegment != NULL) {
        if (currentMemorySegment->NextFreeSegment->PreviousFreeSegment < currentMemorySegment)
            currentMemorySegment->NextFreeSegment->PreviousFreeSegment = currentMemorySegment;
    }
    if (currentMemorySegment->PreviousFreeSegment != NULL) {
        if (currentMemorySegment->PreviousFreeSegment->NextFreeSegment > currentMemorySegment)
            currentMemorySegment->PreviousFreeSegment->NextFreeSegment = currentMemorySegment;
    }
}