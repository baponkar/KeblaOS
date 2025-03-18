# 🖥️ KeblaOS

![KeblaOS Icon](image/KeblaOS.png)


<table>
  <tr><td colspan="2" align="left"><em>Version - 0.12</em></td></tr>
  <tr><td><em>Architecture:</em></td><td><em>x86</em></td></tr>
  <tr><td><em>Bit</em></td><td><em>64</em></td></tr>
  <tr><td><em>Build Date:</em></td><td><code>16.03.2024</code></td></tr>
</table>

----



## Features : This version have following features :

- [x] Limine Bootloader
    * Getting Various Boot Information
    * Getting Firmware Information

- [x] VGA Framebuffer Driver

- [x] ACPI
    * RSDT
    * FADT
    * MADT
    * MCFG
    * HPET

- [x] AHCI

- [x] CPU information and control
    * CPUID
    * SMP

- [x] GDT (Multi Core Support)
- [x] TSS (Multi Core Support)

- [x] Interrupt
    * APIC (Multi Core Support)
    * PIC (Multi Core Support)
    * ISR
    * IRQ

- [x] PCI

- [x] Memory Management
    * Parsing Memory Info
    * 4 Level Paging
    * PMM
    * Kmalloc
    * VMM
    * KHEAP
    * Umalloc
    * UHEAP

- [x] Drivers
    * Disk
    * VGA FRAMEBUFFER
    * I/O PORTS
    * SERIAL
    * Keyboard
    * Speaker
    * Mouse

- [x] Timer
    * TSC
    * RTC
    * PIT
    * APIC
    * HPET

- [x] Multitasking
    * Process
    * Thread
    * Scheduler
    * Set CPU State by regisers

- [x] Simple User Shell

- [ ] Future Plans
    * Multitasking by enabling all cores
    * FAT32 File System
    * Usermode
    * System Call
    * Nice VGA Driver with GPU

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





`src` directory is containing source code. `build` directory is containing generated object file, binary file and iso file. `iso_root` is required for building `image.iso` file.

To build and run by QEmu iso `make -B`.

Downloaded from [here](https://github.com/baponkar/KeblaOS).


