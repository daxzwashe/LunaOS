
LunaOS is an educational x86-64 kernel written in C and assembly. The project is designed for learning operating system development fundamentals: interrupt handling, memory management, device drivers, and basic user input/output.

Features
Loading and Segmentation: GDT with 64-bit code and data segments

Interrupts: IDT, keyboard (IRQ1) and mouse (IRQ12) handlers, PIC remapping

Graphics & Input:

Framebuffer with resolution obtained from UEFI

8x16 font (ASCII 32–126)

Software-rendered mouse cursor with movement and click handling

Memory:

Physical Memory Manager (PMM) using a bitmap (UEFI memory map)

Dynamic heap with linked lists

malloc / free with alignment

PCI: Bus scanning, SATA AHCI controller detection, BAR5 reading

Command Shell: Built-in commands help, clear, memory, malloc, pci

Running in QEMU (Example)
bash
# Build (depends on your toolchain)
make

# Run with OVMF (UEFI)
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -kernel kernel.elf

# Or with a pre-built ISO
qemu-system-x86_64 -cdrom LunaOS.iso
After boot, you'll see the LunaOS> prompt.
Commands: help, clear, memory, malloc, pci.

What You Can Learn
How GDT and IDT work in x86-64

Programmable Interrupt Controller (PIC) remapping

PS/2 keyboard and mouse programming

Physical memory management (bitmap)

Simple dynamic memory allocator

PCI Configuration Space access

Framebuffer output and font rendering

Requirements
x86-64 compiler (gcc / clang)

QEMU (for testing)

UEFI-compatible bootloader (e.g., Limine or custom)

License
MIT — feel free to use for learning, but credit the author if you do.

Contributions
Pull requests and bug reports are welcome.
This is an educational project — improvements to code or documentation help beginners.
