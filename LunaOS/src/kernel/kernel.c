#include "../include/framebuffer.h"
#include "../include/memory.h"
#include "../include/io.h"

// Внешние функции
extern const uint8_t font8x16[];
extern const uint8_t* GetCharBitmap(char c); 
extern void InitGDT();
extern void InitIDT();
extern void InitMouse();

extern void PMM_Init(MemoryInfo* memInfo);
extern uint64_t PMM_GetFreeMemory();
extern void* PMM_AllocPage();
extern void PMM_FreePage(void* address);

// Новые функции
extern void Heap_Init(void* startAddress, size_t sizeBytes);
extern void* malloc(size_t size);
extern void free(void* ptr);
extern void PCI_Scan();

// Глобальные
Framebuffer* GlobalFB;
uint32_t CursorX = 0, CursorY = 0;
const uint32_t FontWidth = 8;
const uint32_t FontHeight = 16;
uint32_t ColorFG = 0xFFFFFFFF; 
uint32_t ColorBG = 0xFF000000; 

char CommandBuffer[128];
int BufferIndex = 0;

// Мышь
uint32_t MouseBackground[256]; 
int OldMouseX = 0, OldMouseY = 0, MouseDrawn = 0;
uint8_t MouseCursorIcon[] = { 2,2,0,0,0,0,0,0,0,0, 2,1,2,0,0,0,0,0,0,0, 2,1,1,2,0,0,0,0,0,0, 2,1,1,1,2,0,0,0,0,0, 2,1,1,1,1,2,0,0,0,0, 2,1,1,1,1,1,2,0,0,0, 2,1,1,1,1,1,1,2,0,0, 2,1,1,2,2,2,2,2,2,0, 2,1,2,0,0,0,0,0,0,0, 2,2,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0 };

void EraseMouseCursor() {
    if (!MouseDrawn) return;
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 10; j++) {
            if ((OldMouseX + j) >= GlobalFB->Width || (OldMouseY + i) >= GlobalFB->Height) continue;
            uint32_t offset = (OldMouseY + i) * GlobalFB->PixelsPerScanLine + (OldMouseX + j);
            screen[offset] = MouseBackground[i * 10 + j];
        }
    }
    MouseDrawn = 0;
}
void DrawMouseCursor(int x, int y) {
    if (MouseDrawn) EraseMouseCursor();
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    OldMouseX = x; OldMouseY = y;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 10; j++) {
            if ((x + j) >= GlobalFB->Width || (y + i) >= GlobalFB->Height) continue;
            uint32_t offset = (y + i) * GlobalFB->PixelsPerScanLine + (x + j);
            MouseBackground[i * 10 + j] = screen[offset];
            uint8_t pixel = MouseCursorIcon[i * 10 + j];
            if (pixel == 1) screen[offset] = 0xFFFFFFFF;
            else if (pixel == 2) screen[offset] = 0xFF000000;
        }
    }
    MouseDrawn = 1;
}

// Утилиты
int strcmp(const char* s1, const char* s2) { while (*s1 && (*s1 == *s2)) { s1++; s2++; } return *(const unsigned char*)s1 - *(const unsigned char*)s2; }
void ClearBuffer() { for(int i=0; i<128; i++) CommandBuffer[i] = 0; BufferIndex = 0; }

// Консоль
void ClearScreen(uint32_t color) {
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    uint64_t total = GlobalFB->Height * GlobalFB->PixelsPerScanLine;
    for (uint64_t i = 0; i < total; i++) screen[i] = color;
    CursorX = 0; CursorY = 0;
}
void ScrollUp() {
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    uint32_t stride = GlobalFB->PixelsPerScanLine;
    int wasDrawn = MouseDrawn;
    if (wasDrawn) EraseMouseCursor();
    for (uint64_t i = 0; i < (GlobalFB->Height - FontHeight) * stride; i++) screen[i] = screen[i + (FontHeight * stride)];
    for (uint64_t i = (GlobalFB->Height - FontHeight) * stride; i < GlobalFB->Height * stride; i++) screen[i] = ColorBG;
    CursorY -= FontHeight;
    if (wasDrawn) DrawMouseCursor(OldMouseX, OldMouseY);
}
void NextLine() { CursorX = 0; CursorY += FontHeight; if (CursorY + FontHeight >= GlobalFB->Height) ScrollUp(); }
void PutChar(char c) {
    if (c == '\n') { NextLine(); return; }
    uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
    const uint8_t* bitmap = GetCharBitmap(c);
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 8; x++) {
            uint32_t offset = (CursorY + y) * GlobalFB->PixelsPerScanLine + (CursorX + x);
            if ((bitmap[y] >> (7 - x)) & 1) screen[offset] = ColorFG; else screen[offset] = ColorBG;
        }
    }
    CursorX += FontWidth;
    if (CursorX + FontWidth >= GlobalFB->Width) NextLine();
}
void kprint(char* str) { while (*str) { PutChar(*str); str++; } }
void kprintInt(int num) { char b[20]; int i=0; if(num==0){kprint("0");return;} if(num<0){kprint("-");num=-num;} while(num!=0){b[i++]=(num%10)+'0';num/=10;} while(i>0)PutChar(b[--i]); }
void kprintHex(uint64_t num) {
    kprint("0x");
    char buffer[20]; int i = 0;
    if (num == 0) { kprint("0"); return; }
    while (num != 0) {
        int rem = num % 16;
        if (rem < 10) buffer[i++] = rem + '0'; else buffer[i++] = rem - 10 + 'A';
        num = num / 16;
    }
    while (i > 0) PutChar(buffer[--i]);
}

// Shell
void OnBackspace() {
    if (BufferIndex > 0) {
        BufferIndex--; CommandBuffer[BufferIndex] = 0;
        CursorX -= FontWidth;
        uint32_t* screen = (uint32_t*)GlobalFB->BaseAddress;
        for(int y=0; y<16; y++) for(int x=0; x<8; x++) screen[(CursorY + y) * GlobalFB->PixelsPerScanLine + (CursorX + x)] = ColorBG;
    }
}
void ExecuteCommand() {
    kprint("\n");
    if (strcmp(CommandBuffer, "help") == 0) kprint("Commands: help, clear, info, memory, malloc, pci");
    else if (strcmp(CommandBuffer, "clear") == 0) { ClearScreen(ColorBG); kprint("LunaOS Kernel v1.6\n"); }
    else if (strcmp(CommandBuffer, "memory") == 0) {
        kprint("Free RAM: "); kprintInt(PMM_GetFreeMemory() / 1024 / 1024); kprint(" MB\n");
    }
    else if (strcmp(CommandBuffer, "malloc") == 0) {
        kprint("Alloc 32 bytes... ");
        char* ptr = (char*)malloc(32);
        kprintHex((uint64_t)ptr); kprint("\n");
        free(ptr);
    }
    else if (strcmp(CommandBuffer, "pci") == 0) {
        PCI_Scan();
    }
    else kprint("Unknown command.");
    ClearBuffer();
    kprint("\nLunaOS> ");
}
void OnKeyPress(char c) {
    if (c == '\n') ExecuteCommand();
    else if (BufferIndex < 127) { CommandBuffer[BufferIndex++] = c; CommandBuffer[BufferIndex] = 0; char t[2]={c,0}; kprint(t); }
}

void KernelStart(BootInfo* bootInfo) {
    GlobalFB = bootInfo->fb;
    ClearScreen(ColorBG);
    kprint("[ OK ] GDT Loaded\n"); InitGDT();
    kprint("[ OK ] IDT Loaded\n"); InitIDT();
    kprint("[ OK ] Mouse Loaded\n"); InitMouse();
    kprint("[ .. ] PMM Init... "); PMM_Init(bootInfo->memInfo); kprint("[ OK ]\n");
    
    // HEAP Init
    kprint("[ .. ] Heap Init... ");
    void* heapStart = (void*)0x10000000;
    Heap_Init(heapStart, 100 * 1024 * 1024);
    kprint("[ OK ]\n");

    kprint("\nWelcome to LunaOS.\nType 'pci' to scan hardware.\n\n");
    kprint("LunaOS> ");
    while(1) { __asm__ volatile("hlt"); }
}