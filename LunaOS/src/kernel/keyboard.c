#include <stdint.h>
#include "../include/io.h"

// Внешняя функция ядра для обработки ввода
extern void OnKeyPress(char c);
extern void OnBackspace();

char ScanCodeLookup[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void HandleKeyboard(void) {
    uint8_t scancode = inb(0x60);

    if (scancode < 0x80) {
        // Enter
        if (scancode == 0x1C) {
            OnKeyPress('\n');
            return;
        }

        // Backspace (0x0E)
        if (scancode == 0x0E) {
            OnBackspace();
            return;
        }

        // Обычные символы
        if (scancode < sizeof(ScanCodeLookup)) {
            char c = ScanCodeLookup[scancode];
            if (c != 0) {
                OnKeyPress(c);
            }
        }
    }
}