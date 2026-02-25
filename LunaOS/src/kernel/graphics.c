#include "../include/framebuffer.h"

extern Framebuffer* GlobalFB;

// Рисуем закрашенный прямоугольник
void DrawRect(int x, int y, int w, int h, uint32_t color) {
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            // Проверка границ экрана
            if ((x + j) >= GlobalFB->Width || (y + i) >= GlobalFB->Height) continue;
            
            uint32_t offset = (y + i) * GlobalFB->PixelsPerScanLine + (x + j);
            screen[offset] = color;
        }
    }
}

// Рисуем пустую рамку
void DrawRectOutline(int x, int y, int w, int h, uint32_t color) {
    DrawRect(x, y, w, 1, color);             // Верх
    DrawRect(x, y + h - 1, w, 1, color);     // Низ
    DrawRect(x, y, 1, h, color);             // Лево
    DrawRect(x + w - 1, y, 1, h, color);     // Право
}