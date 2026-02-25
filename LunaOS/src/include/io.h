#pragma once
#include <stdint.h>

// === 8-битные операции (Клавиатура, Мышь, PIC) ===

// Записать байт (out byte)
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Прочитать байт (in byte)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// === 32-битные операции (PCI, Шина данных) ===

// Записать двойное слово (out long)
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

// Прочитать двойное слово (in long)
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// === Утилиты ===

// Небольшая задержка
static inline void io_wait(void) {
    outb(0x80, 0);
}