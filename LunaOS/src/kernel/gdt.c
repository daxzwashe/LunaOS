#include <stdint.h>

struct GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed));

struct GDTEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t Access;
    uint8_t Limit1_Flags;
    uint8_t Base2;
} __attribute__((packed));

struct GDTEntry DefaultGDT[3];
struct GDTDescriptor GDTR;

// Функция для перезагрузки сегментов (Написана на ассемблере внутри C)
// Она говорит процессору: "Забудь старые настройки UEFI, используй мои!"
void LoadGDT(struct GDTDescriptor* gdt) {
    __asm__ volatile (
        "lgdt (%0)\n\t"         // Загружаем таблицу
        "mov $0x10, %%ax\n\t"   // 0x10 - это наш Data Segment (индекс 2 в таблице)
        "mov %%ax, %%ds\n\t"    // Обновляем Data Segment
        "mov %%ax, %%es\n\t"    // Обновляем Extra Segment
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"    // Обновляем Stack Segment (ОЧЕНЬ ВАЖНО для прерываний)
        
        // Дальше трюк для обновления Code Segment (CS)
        // Мы делаем дальний прыжок (Far Jump) сами на себя
        "pushq $0x08\n\t"       // 0x08 - это наш Code Segment
        "leaq .reload_cs(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"             // Эмулируем дальний возврат
        ".reload_cs:\n\t"
        : 
        : "r" (gdt) 
        : "rax"
    );
}

void InitGDT() {
    // 0: Null Descriptor
    DefaultGDT[0] = (struct GDTEntry){0, 0, 0, 0, 0, 0};

    // 1: Kernel Code Segment (0x08)
    // Access: 0x9A (Executable, Readable, Ring 0)
    // Flags: 0xA0 (64-bit Long Mode)
    DefaultGDT[1] = (struct GDTEntry){0, 0, 0, 0x9A, 0xA0, 0};

    // 2: Kernel Data Segment (0x10)
    // Access: 0x92 (Data, Writable, Ring 0)
    DefaultGDT[2] = (struct GDTEntry){0, 0, 0, 0x92, 0xA0, 0};

    GDTR.Size = sizeof(DefaultGDT) - 1;
    GDTR.Offset = (uint64_t)&DefaultGDT;

    // Вызываем нашу новую функцию с перезагрузкой регистров
    LoadGDT(&GDTR);
}