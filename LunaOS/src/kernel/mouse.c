#include "../include/io.h"
#include <stdint.h>

// Глобальные переменные мыши
int MouseX = 100;
int MouseY = 100;
uint8_t MouseCycle = 0;     // Сщетчик байтов в пакете
uint8_t MousePacket[3];     // Буфер для пакета (3 байта)

// Внешние функции (из kernel.c)
extern void DrawMouseCursor(int x, int y);
extern void EraseMouseCursor();

// Ожидание готовности контроллера (0x64)
void MouseWait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { // Ждем, пока буфер ввода освободится
            if ((inb(0x64) & 1) == 1) return;
        }
        return;
    } else {
        while (timeout--) { // Ждем, пока буфер вывода освободится
            if ((inb(0x64) & 2) == 0) return;
        }
        return;
    }
}

// Запись команды мыши
void MouseWrite(uint8_t write) {
    MouseWait(1);
    outb(0x64, 0xD4); // Команда "Следующий байт - для мыши"
    MouseWait(1);
    outb(0x60, write);
}

// Чтение данных мыши
uint8_t MouseRead() {
    MouseWait(0);
    return inb(0x60);
}

// Инициализация мыши
void InitMouse() {
    uint8_t status;

    // Включаем вспомогательное устройство (Mouse)
    MouseWait(1);
    outb(0x64, 0xA8);

    // Получаем текущий статус
    MouseWait(1);
    outb(0x64, 0x20);
    MouseWait(0);
    status = (inb(0x60) | 2); // Включаем IRQ12 (прерывание мыши)

    // Записываем статус обратно
    MouseWait(1);
    outb(0x64, 0x60);
    MouseWait(1);
    outb(0x60, status);

    // Сброс настроек мыши
    MouseWrite(0xF6);
    MouseRead();  // ACK (0xFA)

    // Включаем передачу данных (Enable Scanning)
    MouseWrite(0xF4);
    MouseRead();  // ACK (0xFA)
}

// Обработчик прерывания мыши (вызывается из IDT)
void HandleMouseInterrupt() {
    uint8_t data = inb(0x60);

    // Мышь шлет 3 байта:
    // 1. Flags (кнопки, переполнение, знак X/Y)
    // 2. Delta X (смещение по X)
    // 3. Delta Y (смещение по Y)
    
    switch(MouseCycle) {
        case 0:
            // Бит 3 всегда должен быть 1. Если нет - рассинхрон.
            if ((data & 0x08) == 0) return;
            MousePacket[0] = data;
            MouseCycle++;
            break;
        case 1:
            MousePacket[1] = data;
            MouseCycle++;
            break;
        case 2:
            MousePacket[2] = data;
            MouseCycle = 0; // Пакет собран

            // Стираем старый курсор
            EraseMouseCursor();

            // Данные смещения (signed char, так как может быть отрицательным)
            int8_t x_rel = (int8_t)MousePacket[1];
            int8_t y_rel = (int8_t)MousePacket[2];

            // Обновляем координаты
            MouseX += x_rel;
            MouseY -= y_rel; // У мыши Y инвертирован (вверх = минус)

            // Ограничиваем экраном (хардкод 1024x768 пока)
            if (MouseX < 0) MouseX = 0;
            if (MouseX > 1024) MouseX = 1024; // TODO: Брать из GlobalFB->Width
            if (MouseY < 0) MouseY = 0;
            if (MouseY > 768) MouseY = 768;

            // Рисуем новый курсор
            DrawMouseCursor(MouseX, MouseY);

            // Обработка клика (ЛКМ = бит 0 в первом байте)
            if (MousePacket[0] & 1) {
                // Left Click!
                // Пока ничего не делаем
            }
            break;
    }
}