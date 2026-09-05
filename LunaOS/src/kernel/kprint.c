#include <stdarg.h>
#include "../include/framebuffer.h"

// AI SLOP Я НЕ ШАРЮ В ЭТОМ ПО ЭТОМУ ДЕЛАЛ ИИ

// Объявления функций, которые у вас уже есть
extern void PutChar(char c);
extern void kprintInt(int num);
extern void kprintHex(uint64_t num);

// Наша «настоящая» функция форматированного вывода
void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        // Если встретили символ %, обрабатываем спецификатор формата
        if (*fmt == '%') {
            fmt++; // Переходим к следующему символу спецификатора
            
            switch (*fmt) {
                case 's': { // Вывод строки: %s
                    char* str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    while (*str) {
                        PutChar(*str++);
                    }
                    break;
                }
                case 'd': { // Вывод целого десятичного числа: %d
                    int num = va_arg(args, int);
                    kprintInt(num);
                    break;
                }
                case 'x': { // Вывод шестнадцатеричного числа: %x
                    uint64_t num = va_arg(args, uint64_t);
                    kprintHex(num);
                    break;
                }
                case 'c': { // Вывод одного символа: %c
                    char c = (char)va_arg(args, int);
                    PutChar(c);
                    break;
                }
                case '%': { // Вывод самого знака %
                    PutChar('%');
                    break;
                }
                default: { // Если неизвестный спецификатор
                    PutChar('%');
                    PutChar(*fmt);
                    break;
                }
            }
        } else {
            // Обычный символ — выводим на экран
            PutChar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}