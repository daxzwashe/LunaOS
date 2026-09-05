#include <stdint.h>

static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

// Простая задержка в миллисекундах
void sleep_ms(uint32_t ms) {
    // В QEMU стандартная частота эмуляции ~2 ГГц (2 000 000 тактов в мс)
    uint64_t ticks_per_ms = 2000000; 
    uint64_t start = rdtsc();
    uint64_t wait_ticks = ms * ticks_per_ms;

    while ((rdtsc() - start) < wait_ticks) {
        __asm__ volatile ("pause");
    }
}

// Задержка в секундах
void sleep(uint32_t seconds) {
    sleep_ms(seconds * 1000);
}