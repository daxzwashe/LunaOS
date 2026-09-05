.DEFAULT_GOAL := all

CC ?= clang
LD ?= lld-link
DD ?= dd
MFORMAT ?= mformat
MMD ?= mmd
MCOPY ?= mcopy
MKDIR ?= mkdir
RM ?= rm
QEMU ?= qemu-system-x86_64
CFLAGS = -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -mgeneral-regs-only -I./src/include
LDFLAGS = -subsystem:efi_application -nodefaultlib -dll -entry:efi_main

SRC_DIR = $(CURDIR)/src
BOOTLOADER_DIR = $(SRC_DIR)/bootloader
KERNEL_DIR = $(SRC_DIR)/kernel
TEMP_DIR = $(CURDIR)/temp
BOOTLOADER_TEMP_DIR = $(TEMP_DIR)/bootloader/
KERNEL_TEMP_DIR = $(TEMP_DIR)/kernel/
BUILD_DIR = $(CURDIR)/build

COMMON_OBJS = $(BOOTLOADER_TEMP_DIR)/main.o $(KERNEL_TEMP_DIR)/kernel.o $(KERNEL_TEMP_DIR)/timer.o \
			  $(KERNEL_TEMP_DIR)/kprint.o $(KERNEL_TEMP_DIR)/font.o $(KERNEL_TEMP_DIR)/idt.o \
			  $(KERNEL_TEMP_DIR)/keyboard.o $(KERNEL_TEMP_DIR)/gdt.o $(KERNEL_TEMP_DIR)/mouse.o \
			  $(KERNEL_TEMP_DIR)/graphics.o $(KERNEL_TEMP_DIR)/pmm.o $(KERNEL_TEMP_DIR)/heap.o \
			  $(KERNEL_TEMP_DIR)/pci.o $(KERNEL_TEMP_DIR)/apple.o $(KERNEL_TEMP_DIR)/badapple_data.o

.PHONY: all

all: prepare image

run: all
	$(QEMU) \
	  -m 512M \
	  -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/OVMF.fd \
	  -device ahci,id=ahci \
	  -drive id=disk,file=$(BUILD_DIR)/lunaos.img,if=none,format=raw \
	  -device ide-hd,drive=disk,bus=ahci.0 \
	  -net none

prepare:
	$(MKDIR) -p $(BOOTLOADER_TEMP_DIR)
	$(MKDIR) -p $(KERNEL_TEMP_DIR)
	$(MKDIR) -p $(BUILD_DIR)
	
clean:
	$(RM) -rf $(TEMP_DIR)
	$(RM) -rf $(BUILD_DIR)

image: $(BUILD_DIR)/lunaos.img

$(BUILD_DIR)/lunaos.img: $(TEMP_DIR)/boot.efi
	$(DD) if=/dev/zero of=build/lunaos.img bs=1M count=64 2>/dev/null
	$(MFORMAT) -i $@ -F ::
	$(MMD) -i $@ ::/EFI
	$(MMD) -i $@ ::/EFI/BOOT
	$(MCOPY) -i $@ $(TEMP_DIR)/boot.efi ::/EFI/BOOT/BOOTX64.EFI

$(TEMP_DIR)/boot.efi: $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $^ -out:$@

$(BOOTLOADER_TEMP_DIR)/%.o: $(BOOTLOADER_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_TEMP_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@