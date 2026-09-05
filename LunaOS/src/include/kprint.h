#ifndef KPRINT_H
#define KPRINT_H

#include <stdint.h>

void kprint(const char* str);
void kprintInt(int num);
void kprintHex(uint64_t num);
void kprintf(const char* fmt, ...);

#endif