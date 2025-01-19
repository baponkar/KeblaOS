# KeblaOS

![KeblaOS Icon](./src/img/KeblaOS_icon.bmp)

## Version - 0.8

## Architecture : x86_64

## Build Date : 08/12/2024

## Description : This version just print some bootloader supplied information.

Screenshot 1
![screenshot 1](./screenshot/screenshot_01.png)

Screenshot 2
![screenshot 2](./screenshot/keblaOS_0.7.gif)

## Used Tools Version :
- [x] [Limine Bootloader](https://github.com/limine-bootloader/limine) - 8.6.0
- [x] [x86_64-elf-gcc](https://wiki.osdev.org/GCC_Cross-Compiler) (GCC) 14.2.0
- [x] GNU ld (GNU Binutils) 2.43
- [x] GNU Make 4.3
- [x] bison (GNU Bison) 3.8.2
- [x] flex 2.6.4
- [x] xorriso 1.5.4
- [x] NASM version 2.15.05
- [x] GNU gdb (Ubuntu 12.1-0ubuntu1~22.04.2) 12.1
- [x] 
- [x] [QEMU emulator](https://www.qemu.org/) version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.24)
- [x] [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) Ubuntu 22.04.4 LTS

## Features : This version have following features :

- [x] Limine Bootloader
- [x] VGA Framebuffer Driver
- [x] stdlib.c included in lib directory
- [x] string.c includeed in lib directory
- [x] Global Descriptor Table(GDT)
- [x] Interrupt Descriptor Table(IDT)
- [x] PIT Timer
- [x] Shell
- [x] Speaker Driver
- [x] Keyboard Driver
- [x] Physical Memory Manager(PMM)
- [x] Kernel Heap(kheap)
- [x] Paging
- [x] Process
- [x] Schedular



`src` directory is containing source code. `build` directory is containing generated object file, binary file and iso file. `iso_root` is required for building `image.iso` file.

To build and run by QEmu iso `make -B`.

Downloaded from [here](https://github.com/baponkar/KeblaOS).


