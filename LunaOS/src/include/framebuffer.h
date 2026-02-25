#pragma once
#include <stdint.h>

typedef struct {
    void* BaseAddress;          // Адрес видеопамяти
    uint64_t BufferSize;        // Размер видеопамяти в байтах
    uint32_t Width;             // Ширина экрана (px)
    uint32_t Height;            // Высота экрана (px)
    uint32_t PixelsPerScanLine; // Пикселей в одной строке (иногда больше ширины)
} Framebuffer;