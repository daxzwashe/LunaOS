#!/bin/bash
set -e 

CC="clang"
LD="lld-link"
CFLAGS="-target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -mgeneral-regs-only -I./src/include"
LDFLAGS="-subsystem:efi_application -nodefaultlib -dll -entry:efi_main"

mkdir -p build
mkdir -p dist/EFI/BOOT

echo ">>> COMPILING <<<"
# Компилируем все файлы
$CC $CFLAGS -c src/bootloader/main.c -o build/main.o
$CC $CFLAGS -c src/kernel/kernel.c -o build/kernel.o
$CC $CFLAGS -c src/kernel/font.c -o build/font.o
$CC $CFLAGS -c src/kernel/idt.c -o build/idt.o
$CC $CFLAGS -c src/kernel/keyboard.c -o build/keyboard.o
$CC $CFLAGS -c src/kernel/gdt.c -o build/gdt.o
$CC $CFLAGS -c src/kernel/mouse.c -o build/mouse.o
$CC $CFLAGS -c src/kernel/graphics.c -o build/graphics.o
$CC $CFLAGS -c src/kernel/pmm.c -o build/pmm.o
$CC $CFLAGS -c src/kernel/heap.c -o build/heap.o
$CC $CFLAGS -c src/kernel/pci.c -o build/pci.o

echo ">>> LINKING <<<"
# Линкуем всё вместе
$LD $LDFLAGS build/main.o \
             build/kernel.o \
             build/font.o \
             build/idt.o \
             build/keyboard.o \
             build/gdt.o \
             build/mouse.o \
             build/graphics.o \
             build/pmm.o \
             build/heap.o \
             build/pci.o \
             -out:dist/EFI/BOOT/BOOTX64.EFI

echo ">>> IMAGE <<<"
dd if=/dev/zero of=build/lunaos.img bs=1M count=64 2>/dev/null
mformat -i build/lunaos.img -F ::
mmd -i build/lunaos.img ::/EFI
mmd -i build/lunaos.img ::/EFI/BOOT
mcopy -i build/lunaos.img dist/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

echo ">>> RUN <<<"
qemu-system-x86_64 \
  -m 512M \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/OVMF.fd \
  -device ahci,id=ahci \
  -drive id=disk,file=build/lunaos.img,if=none,format=raw \
  -device ide-hd,drive=disk,bus=ahci.0 \
  -net none