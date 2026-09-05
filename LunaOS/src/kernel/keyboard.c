#include <stdint.h>
#include "../include/io.h"

extern void OnKeyPress(char c);
extern void OnBackspace();

// Флаг остановки воспроизведения Bad Apple
volatile int StopVideoFlag = 0;
static int CtrlPressed = 0;

char ScanCodeLookup[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void HandleKeyboard(void) {
    uint8_t scancode = inb(0x60);

    // Обработка зажатия/отпускания Left Ctrl (0x1D)
    if (scancode == 0x1D) {
        CtrlPressed = 1;
        return;
    } else if (scancode == (0x1D | 0x80)) {
        CtrlPressed = 0;
        return;
    }

    // Нажатие клавиши
    if (scancode < 0x80) {
        // Проверка Ctrl + C (Скан-код 'c' = 0x2E)
        if (CtrlPressed && scancode == 0x2E) {
            StopVideoFlag = 1;
            return;
        }

        if (scancode == 0x1C) { // Enter
            OnKeyPress('\n');
            return;
        }

        if (scancode == 0x0E) { // Backspace
            OnBackspace();
            return;
        }

        if (scancode < sizeof(ScanCodeLookup)) {
            char c = ScanCodeLookup[scancode];
            if (c != 0) {
                OnKeyPress(c);
            }
        }
    }
}