#include <stdint.h>
#include "../include/io.h"

// Структуры
struct IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t types_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct IDTR {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct IDTEntry IDT[256];
struct IDTR idtr;

// Внешние функции обработчики
extern void HandleKeyboard(void);
extern void HandleMouseInterrupt(void);

// === ISR: KEYBOARD (IRQ 1) ===
__attribute__((interrupt)) void KeyboardISR(void* frame) {
    HandleKeyboard();
    outb(0x20, 0x20); // Сообщаем Master PIC, что закончили
}

// === ISR: MOUSE (IRQ 12) ===
__attribute__((interrupt)) void MouseISR(void* frame) {
    HandleMouseInterrupt();
    outb(0xA0, 0x20); // Сообщаем Slave PIC
    outb(0x20, 0x20); // Сообщаем Master PIC
}

// Перенастройка контроллера прерываний
void RemapPIC() {
    uint8_t a1, a2;

    a1 = inb(0x21);
    a2 = inb(0xA1);

    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    outb(0x21, 0x20); io_wait(); // Master -> 32
    outb(0xA1, 0x28); io_wait(); // Slave -> 40

    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // Маски: IRQ1 (Keyboard) и IRQ12 (Mouse)
    outb(0x21, 0xF9); 
    outb(0xA1, 0xEF);
}

// Инициализация
void InitIDT() {
    // 1. Клавиатура (IRQ 1 -> Вектор 33)
    uint64_t kbd_offset = (uint64_t)KeyboardISR;
    IDT[33].offset_low = kbd_offset & 0xFFFF;
    IDT[33].selector = 0x08;
    IDT[33].ist = 0;
    IDT[33].types_attr = 0x8E;
    IDT[33].offset_mid = (kbd_offset >> 16) & 0xFFFF;
    IDT[33].offset_high = (kbd_offset >> 32) & 0xFFFFFFFF;
    IDT[33].zero = 0;

    // 2. Мышь (IRQ 12 -> Вектор 44)
    uint64_t mouse_offset = (uint64_t)MouseISR;
    IDT[44].offset_low = mouse_offset & 0xFFFF;
    IDT[44].selector = 0x08;
    IDT[44].ist = 0;
    IDT[44].types_attr = 0x8E;
    IDT[44].offset_mid = (mouse_offset >> 16) & 0xFFFF;
    IDT[44].offset_high = (mouse_offset >> 32) & 0xFFFFFFFF;
    IDT[44].zero = 0;

    RemapPIC();
    
    idtr.limit = sizeof(IDT) - 1;
    idtr.offset = (uint64_t)&IDT;

    __asm__ volatile ("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");
}