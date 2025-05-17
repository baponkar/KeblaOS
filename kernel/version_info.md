# 🖥️ KeblaOS

![KeblaOS Icon](image/KeblaOS.png)


<table>
  <tr><td colspan="2" align="left"><em>Version - 0.14.2</em></td></tr>
  <tr><td><em>Architecture:</em></td><td><em>x86</em></td></tr>
  <tr><td><em>Bit</em></td><td><em>64</em></td></tr>
  <tr><td><em>Build Date:</em></td><td><code>05.05.2025</code></td></tr>
</table>

----



## Features : This version have following features :

- [x] Limine Bootloader
    * Getting Various Boot Information
    * Getting Firmware Information

- [x] VGA Framebuffer Driver
    * Additional functions

- [x] ACPI
    * RSDT
    * FADT
    * MADT
    * MCFG
    * HPET

- [x] AHCI
    * AHCI SATA Disk Driver

- [x] CPU information and control
    * CPUID
    * SMP

- [x] GDT (Multi Core Support)
- [x] TSS (Multi Core Support)

- [x] Interrupt
    * APIC (Multi Core Support)
    * PIC
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

- [x] Simple Interactive Kernel Shell (kshell)
    * Calculator
    * Steam Locomotive animation

- [x] Multitasking
    * Multitasking by enabling all cores

- [x] Filesystem
    * FAT32 File System

- [x] Interrupt Based System Call (Software Interrupt)

- [x] Switching into User Mode
    * MSR Based System Call
    * Interrupt Based System Call
    * Loading ELF, Binary File

```bash
[Info] Serial: Successfully Serial read write enabled.
 [*] Limine - 9.2.3 bootloader.
[Info] KeblaOS - 0.14.2
[Info] Build starts on: 26/01/2025, Last Update on: 05/05/2025
 [*] CPUID: CPU Brand : QEMU Virtual CPU version 2.5+
 [*] CPUID: CPU Vendor : AuthenticAMD���
 [*] CPUID: CPU Base Frequency: 0 Hz
 [-] Memory: Start Stack size : 0x4000
 [-] Memory: LIMINE_PAGING_MODE_X86_64_4LVL
 [-] Memory: Kernel position address: Virtual = 0xFFFFFFFF80000000, Physical = 0x7EBD3000
 [-] Memory: HHDM Offset: 0xFFFF800000000000
 [-] Memory: Higher Half Start(Virtual): 0xFFFF800000001000 and End(Virtual): 0xFFFFFFFFFFFFFFFF
 [-] Memory: Higher Half Start(Physical): 0x1000 and End(Physical): 0x7FFFFFFFFFFF
 [-] Memory: Physical Base: 0x1000, Length: 0x52000, Type: 5
 [-] Memory: Physical Base: 0x53000, Length: 0x4C000, Type: 0
 [-] Memory: Physical Base: 0x9FC00, Length: 0x400, Type: 1
 [-] Memory: Physical Base: 0xF0000, Length: 0x10000, Type: 1
 [-] Memory: Physical Base: 0x100000, Length: 0x7EAD3000, Type: 0
 [-] Memory: Physical Base: 0x7EBD3000, Length: 0x7AC000, Type: 6
 [-] Memory: Physical Base: 0x7F37F000, Length: 0x5CE000, Type: 5
 [-] Memory: Physical Base: 0x7F94D000, Length: 0x5B4000, Type: 0
 [-] Memory: Physical Base: 0x7FF01000, Length: 0xDE000, Type: 5
 [-] Memory: Physical Base: 0x7FFDF000, Length: 0x21000, Type: 1
 [-] Memory: Physical Base: 0xB0000000, Length: 0x10000000, Type: 1
 [-] Memory: Physical Base: 0xFD000000, Length: 0x300000, Type: 7
 [-] Memory: Physical Base: 0xFED1C000, Length: 0x4000, Type: 1
 [-] Memory: Physical Base: 0xFFFC0000, Length: 0x40000, Type: 1
 [-] Memory: Physical Base: 0x100000000, Length: 0x80000000, Type: 0
 [-] Memory: Usable Phys. memory => Start: 0x100000000, End: 0x180000000, Length: 0x180000000
 [-] Memory: Total Device Memory = 0x1102F3400
[Info] SMP: Flags: 0x0, bsp_lapic_id: 0, cpu_count: 4
 [-] cpu_id: 0, lapic_id: 0, reserved: 0x0, goto_address: 0x0, extra_argument: 0x0
 [-] cpu_id: 1, lapic_id: 1, reserved: 0xFFFF80007FF3C000, goto_address: 0x0, extra_argument: 0x0
 [-] cpu_id: 2, lapic_id: 2, reserved: 0xFFFF80007FF2C000, goto_address: 0x0, extra_argument: 0x0
 [-] cpu_id: 3, lapic_id: 3, reserved: 0xFFFF80007FF1C000, goto_address: 0x0, extra_argument: 0x0
 [-] GDT & TSS initialized.
 [-] Successfully initialized PMM!
 [-] Successfully Paging initialized.
 [+] PIC Enabled.
[Info] Successfully pic Bootstrap Interrupt Initialized.
[Info] PIT Timer initialized with 100 ms interval (divisor: 53782, frequency: 10 Hz)
PIT Tick no : 10
 [-] TSC Timer initialized with CPU Frequency 708863209 Hz
[Info] CPU 0 with PIC initialized...

 [-] PIC Disabled.
 [-] Memory: Start Stack size : 0x4000
 [-] Memory: LIMINE_PAGING_MODE_X86_64_4LVL
 [-] Memory: Kernel position address: Virtual = 0xFFFFFFFF80000000, Physical = 0x7EBD3000
 [-] Memory: HHDM Offset: 0xFFFF800000000000
 [-] Memory: Higher Half Start(Virtual): 0xFFFF800000001000 and End(Virtual): 0xFFFFFFFFFFFFFFFF
 [-] Memory: Higher Half Start(Physical): 0x1000 and End(Physical): 0x7FFFFFFFFFFF
 [-] Memory: Physical Base: 0x1000, Length: 0x52000, Type: 5
 [-] Memory: Physical Base: 0x53000, Length: 0x4C000, Type: 0
 [-] Memory: Physical Base: 0x9FC00, Length: 0x400, Type: 1
 [-] Memory: Physical Base: 0xF0000, Length: 0x10000, Type: 1
 [-] Memory: Physical Base: 0x100000, Length: 0x7EAD3000, Type: 0
 [-] Memory: Physical Base: 0x7EBD3000, Length: 0x7AC000, Type: 6
 [-] Memory: Physical Base: 0x7F37F000, Length: 0x5CE000, Type: 5
 [-] Memory: Physical Base: 0x7F94D000, Length: 0x5B4000, Type: 0
 [-] Memory: Physical Base: 0x7FF01000, Length: 0xDE000, Type: 5
 [-] Memory: Physical Base: 0x7FFDF000, Length: 0x21000, Type: 1
 [-] Memory: Physical Base: 0xB0000000, Length: 0x10000000, Type: 1
 [-] Memory: Physical Base: 0xFD000000, Length: 0x300000, Type: 7
 [-] Memory: Physical Base: 0xFED1C000, Length: 0x4000, Type: 1
 [-] Memory: Physical Base: 0xFFFC0000, Length: 0x40000, Type: 1
 [-] Memory: Physical Base: 0x100000000, Length: 0x80000000, Type: 0
 [-] Memory: Usable Phys. memory => Start: 0x100000000, End: 0x180000000, Length: 0x180000000
 [-] Memory: Total Device Memory = 0x1102F3400
 [-] ACPI 1.0 is signature and checksum validated
[Error] FADT not found! ACPI status unknown.
 [-] Found I/O APIC: ID = 0, Address = 0xFEC00000, GSI Base = 0
 [-] Successfully ACPI Enabled
 [-] Found I/O APIC: ID = 0, Address = 0xFEC00000, GSI Base = 0
 [-] Successfully Paging initialized.
 [-] Successfully initialized PMM!
 [-] GDT & TSS initialized.
 [-] IOAPIC Hardware IRQs routed to LAPIC ID 0
 [-] Successfully Bootstrap apic Interrupt Initialized.
 [-] Interrupt Based System Call initialized!
[Error] FPU not present!
 [-] APIC Timer Frequency: 27615 ticks/ms
 [-] APIC Timer calibrated with 27615 ticks/ms
 [-] APIC Timer initialized with 100 ms interval in CPU: 0.
 [-] Successfully KEYBOARD initialized.
[Info] Bootstrap CPU 0 initialized...

 [-] Memory: Start Stack size : 0x4000
 [-] Memory: LIMINE_PAGING_MODE_X86_64_4LVL
 [-] Memory: Kernel position address: Virtual = 0xFFFFFFFF80000000, Physical = 0x7EBD3000
 [-] Memory: HHDM Offset: 0xFFFF800000000000
 [-] Memory: Higher Half Start(Virtual): 0xFFFF800000001000 and End(Virtual): 0xFFFFFFFFFFFFFFFF
 [-] Memory: Higher Half Start(Physical): 0x1000 and End(Physical): 0x7FFFFFFFFFFF
 [-] Memory: Physical Base: 0x1000, Length: 0x52000, Type: 5
 [-] Memory: Physical Base: 0x53000, Length: 0x4C000, Type: 0
 [-] Memory: Physical Base: 0x9FC00, Length: 0x400, Type: 1
 [-] Memory: Physical Base: 0xF0000, Length: 0x10000, Type: 1
 [-] Memory: Physical Base: 0x100000, Length: 0x7EAD3000, Type: 0
 [-] Memory: Physical Base: 0x7EBD3000, Length: 0x7AC000, Type: 6
 [-] Memory: Physical Base: 0x7F37F000, Length: 0x5CE000, Type: 5
[Info] AP CPU core 1 started...
 [-] Memory: Physical Base: 0x7F94D000, Length: 0x5B4000, Type: 0
 [-] Memory: Start Stack size : 0x4000
 [-] Memory: Physical Base: 0x7FF01000, Length: 0xDE000, Type: 5
 [-] Memory: LIMINE_PAGING_MODE_X86_64_4LVL
 [-] Memory: Physical Base: 0x7FFDF000, Length: 0x21000, Type: 1
 [-] Memory: Kernel position address: Virtual = 0xFFFFFFFF80000000, Physical = 0x7EBD3000
 [-] Memory: HHDM Offset: 0xFFFF800000000000
 [-] Memory: Physical Base: 0xB0000000, Length: 0x10000000, Type: 1
 [-] Memory: Higher Half Start(Virtual): 0xFFFF800000001000 and End(Virtual): 0xFFFFFFFFFFFFFFFF
 [-] Memory: Physical Base: 0xFD000000, Length: 0x300000, Type: 7
[Info] AP CPU core 2 started...
 [-] Memory: Higher Half Start(Physical): 0x1000 and End(Physical): 0x7FFFFFFFFFFF
 [-] Memory: Physical Base: 0xFED1C000, Length: 0x4000, Type: 1
 [-] Memory: Physical Base: 0x1000, Length: 0x52000, Type: 5
 [-] Memory: Start Stack size : 0x4000
 [-] Memory: Physical Base: 0x53000, Length: 0x4C000, Type: 0
 [-] Memory: Physical Base: 0xFFFC0000, Length: 0x40000, Type: 1
 [-] Memory: Physical Base: 0x9FC00, Length: 0x400, Type: 1
 [-] Memory: LIMINE_PAGING_MODE_X86_64_4LVL
 [-] Memory: Physical Base: 0x100000000, Length: 0x80000000, Type: 0
 [-] Memory: Physical Base: 0xF0000, Length: 0x10000, Type: 1
 [-] Memory: Usable Phys. memory => Start: 0x100000000, End: 0x180000000, Length: 0x180000000
 [-] Memory: Physical Base: 0x100000, Length: 0x7EAD3000, Type: 0
 [-] Memory: Total Device Memory = 0x1102F3400
 [-] Memory: Kernel position address: Virtual = 0xFFFFFFFF80000000, Physical = 0x7EBD3000
 [-] Successfully initialized PMM!
 [-] Memory: Physical Base: 0x7EBD3000, Length: 0x7AC000, Type: 6
 [-] CPU 1: Set CR3 to PML4 address: 0x7FF58000
 [-] Memory: HHDM Offset: 0xFF [-] MemoryF:F80 Physical Bas0e:0000 0x7F37F000, Length: 000000
x5CE000, Type: 5
 [-] Successfully Paging initialized for core 1.
 [-] Memory: Physical Base: 0x7F94D000, Length: 0x5B4000, Type: 0
 [-] Memory: Higher Half Start(Virtual): 0xFFFF800000001000 and End(Virtual) [-] CPU 1: Set CR3 to PML4 addres: 0xFFFFFFFFFFFFFFFF
s [-] Memory: Physical Base: 0x7FF01000, Length: 0xDE000, Type: 5
: 0x7FF58000
 [-] Memory: Higher Half Start(Physical): 0x1000 and End(Physical): 0x7FFFFFFFFFFF
[Info] AP CPU [-] Memory core 3 start: Phyesical Based: 0x7FFDF000, ...
L [-] Successfully Paging initialized for core e1n.
gth: 0x21000, Type: 1
 [-] Memory: Physical Base: 0x1000, Length: 0x52000, Type: 5
 [-] CPU 0 is online
 [-] Initialize GDT & TSS for CPU 1.
 [-] Memory: Physical Base: 0xB0000000, Length: 0x10000000, Type: 1
 [-] Memory: Physical Base: 0x53000, Length: 0x4C000, Type: 0
 [-] Memory: Physical Base: 0xFD000000, Length: 0x300000, Type: 7
  [-][ -]S ucMemory:cessfully CPU 1 Interrupt Initialized.
 Physical Base: 0x9FC00, Length: 0x400, Type: 1
 [-] Memory: Physical Base: 0xFED1C000, Length: 0x4000, Type: 1
 [-] CPU 0 is online
[Error] FPU not present!
 [-] Memory: Physical Base: 0xF0000, Length: 0x1 0000[, Typ-e: 1
] CPU 1 (LAPIC ID: 0x1) is online
 [-] CPU 1 is online
 [-] Memor [-] Memory: Physical y: PhysBase: 0xi100000, Lecal Bangth: 0sx7EADe: 0xFFFC0000, Lengt3h:000,  Type: 0
0 [-] CPU [-] 0 is Mx oemory: Pnlineh400y00
, sTiypce: 1al
 Base: 0x7EBD3000, Length: 0x7AC000, Type: 6
  [-] Mem [-] Moremy:ory P:h yPshiycsail Bacsael:  0Bxa1s0e0:0 000x070F03, 7FL0e00ngth, Leng: 0th: x0x5CE8000000000, Typ0, Type: 0
e[-] CPU 1 is online
 [-] Memory: Usable Phys. memory => Start: 0x100000000, End: 0x180000000, Length: 0x180000000
: 5
 [-] CPU 0 is online
 [-] Memo ry: Physical Ba[-]se Memory: T:o 0xt7al DevFi9ce Memory 4D00=0, Length: 0x5B4 0x110002F0, Type: 0
3400
  [[-] Su-] CPU 1 is online
ccessfully initialized PMM!
 [-] Memory: Physical Base: 0x7FF01000, Length: 0xDE000, Type: 5
 [-] CPU 2: Set CR3 to PML4 address: 0x7FF58000
 [-] Memory: Physical Base: 0x7FFDF000, Length: 0x21000, Type: 1
 [-] CPU 0 is online
 [-] Successfully P [-] Memoaging iry: Pnitializehd forysical core 2.
 Base: 0xB0000000, Length: 0x10000000, Type: 1
 [-] CPU 1 is online
 [-] CPU 2: Set CR3 to PML4 address: 0x7FF58000
 [-] Memory: Physical Base: 0xFD000000, Length: 0x300000, Type: 7
 [-] CPU 0 is online
 [-] Successfully Paging initialized for core 2 [-.
] Memory: Physical Base: 0xFED1C000, Length: 0x4000, Type: 1
 [-] Initialize GDT & TSS for CPU 2.
 [-] CPU 1 is online
 [-] Memory: Physical Base: 0xFFFC0000, Length: 0x40000, Type: 1
 [-] Successfully CPU 2 Interrupt Initialized.
 [-] CPU 0 is online
 [-] Memory: Physical Base: 0x100000000, Length: 0x80000000, Type: 0
[Error] FPU not present!
 [-] Memory: Usable Phys. memory => Start: 0x100000000, End: 0x180000000, Length: 0x180000000
 [-] CPU 1 is online
 [-] [-] C Memory: TotalPU 2 (LAPIC ID: 0x2) is online
 Device Memory = 0x1102F3400
 [-] CPU 2 is online
 [-] Successfully initialized PMM!
 [-] CPU 0 is online
 [-] CPU 3: Set CR3 to PML4 address: 0x7FF58000
 [-] CPU 1 is online
 [-] Successfully Paging initialized for core 3.
 [-] CPU 3: Set CR3 to PML4 address: 0x7FF58000
 [-] CPU 2 is online
 [-] Successfully Paging initialized for core 3.
 [-] CPU 0 is online
 [-] Initialize GDT & TSS for CPU 3.
 [-] CPU 1 is online
 [-] Successfully CPU 3 Interrupt Initialized.
[Error] FPU not present!
 [-] CPU 2 is online
 [-] CPU 3 (LAPIC ID: 0x3) is online
 [-] CPU 3 is online
[Info] All CPU cores initialized and online.
Hello from CPU 0 (BSP)
[INFO] Scanning PCI devices...
 [-] PCI: Detected Network Controller at 0:2.0 - Ethernet Controller
 [-] Device ID: 0x10D3, Vendor ID: 0x8086
 [-] PCI: Detected SATA Disk at 0:31.2 -  Class: 1, Subclass: 6, Prog IF: 1, Revision: 2
 [-] Device ID: 0x2922, Vendor ID: 0x8086
[INFO] PCI Scan Completed
[Info] Start Testing AHCI
 [-] AHCI: Write successful from buf_1 into disk.
 [-] AHCI: Data read from disk: Hello from KeblaOS!.
[Info] AHCI test completed successfully.
[Info] Successfully initialize FAT32 with bytes per sector: 25931
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �EST    TXT  Size: 18
  FAT32: File: �apon.txt Size: 0
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �EST    TXT  Size: 18
  FAT32: File: �apon.txt Size: 0
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �EST    TXT  Size: 18
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �EST    TXT  Size: 18
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �EST    TXT  Size: 18
  FAT32: File: �estfile.tx  Size: 28
  FAT32: File: �estfile.tx  Size: 28
[File-System]: Failed to create file 'testfile.txt'.
 [-] FAT32: Failed to delete file!
[Info] Start Searching Ports
 [-] AHCI: SATA drive found at port: 0
 [-] FAT32: Initializing FAT32...
[Info] Successfully initialize FAT32 with bytes per sector: 25931
 [-] FAT32: FAT32 initialized successfully.
 [-] FAT32: Creating file: TEST    TXT
 [-] FAT32: Failed to create file!
Created Process: Shell Process (PID: 0)
Created Thread: Shell Thread (TID: 0) at 0xFFFF800000015000 | rip : 0xFFFFFFFF80008B21 | rsp : 0xFFFF80000001B000
KeblaOS>>
```

Screenshot 1
![screenshot 1](./screenshot/keblaos_screenshot_1.png)

Screenshot 2
![screenshot 2](./screenshot/keblaos_screenshot_2.png)

Screenshot 3
![screenshot 3](./screenshot/keblaos_screenshot_3.png)

Screenshot 4
![screenshot 2](./screenshot/keblaos_screenshot_4.png)

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
To get Make help by `make help`

Downloaded from [here](https://github.com/baponkar/KeblaOS).


