[![KeblaOS Badge](https://img.shields.io/badge/Kebla-OS-maker?labelColor=red&color=blue)](https://gitlab.com/baponkar/kebla-os)
[![GitHub Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitHub&logoColor=Blue&labelColor=white&color=blue)
](https://github.com/baponkar/KeblaOS)
[![GitLab Badge](https://img.shields.io/badge/Fork-Me-maker?logo=GitLab&logoColor=Blue&labelColor=white&color=blue)
](https://gitlab.com/baponkar/KeblaOS)
[![Linux Badge](https://img.shields.io/badge/-Linux-maker?logo=linux&logoColor=black&logoSize=auto&labelColor=white&color=blue)
](https://kernel.com)
![C Badge](https://img.shields.io/badge/C-Language-maker?logo=c&logoColor=black&labelColor=white&color=blue)
![x86_32bit Badge](https://img.shields.io/badge/x86-32bit-maker?logo=intel&labelColor=white&color=blue)
![ASM Badge](https://img.shields.io/badge/ASM-Language-maker?logo=assembly&labelColor=white&color=blue)
--------------------------------------------------------------------------------------------------------------------


# What Happens When You Power On a Computer: The Boot Process Explained

Have you ever wondered what happens behind the scenes when you press the power button on your computer? This seemingly simple action initiates a complex sequence of events known as the **boot process**, which prepares the system to load the operating system and make the computer ready for use.

In this article, we'll break down the key stages of what happens after you power on a computer.

---

## 1. **Power Supply and Voltage Checks**
When you press the power button:
- The **power supply unit (PSU)** delivers power to the various components of the computer, including the motherboard, CPU, memory, and storage devices.
- The PSU performs a **Power-On Self-Test (POST)** to check if the voltages are correct and stable. If the power levels are sufficient, it signals the motherboard to proceed.

---

## 2. **BIOS/UEFI Initialization**
Once the power supply stabilizes, the **Basic Input/Output System (BIOS)** or **Unified Extensible Firmware Interface (UEFI)** firmware on the motherboard takes control. The BIOS/UEFI is the first software that runs when the computer starts.

### Key Tasks of BIOS/UEFI:
- **POST (Power-On Self-Test)**: The BIOS/UEFI performs another test, this time checking the hardware components like the CPU, RAM, and storage devices. If any critical hardware component is malfunctioning, the POST will halt the boot process and alert the user through a series of beeps or error messages.
- **Hardware Initialization**: The BIOS/UEFI initializes hardware devices like the graphics card, USB ports, and network interfaces. It ensures that all connected devices are ready for communication.
- **Detect Bootable Devices**: The BIOS/UEFI checks the system's boot order (usually stored in non-volatile memory) to determine which device (hard drive, SSD, USB, etc.) contains the boot loader that will load the operating system.

---

## 3. **Boot Loader Execution**
Once the BIOS/UEFI identifies the bootable device, it looks for a program called the **boot loader**. This program resides in the **Master Boot Record (MBR)** or **GUID Partition Table (GPT)**, depending on the storage medium's partitioning system.

### Key Functions of the Boot Loader:
- The boot loader is responsible for loading the operating system kernel into memory.
- Popular boot loaders include **GRUB** (used by Linux), **Windows Boot Manager**, and **LILO**.
- If the boot loader is missing or corrupted, the system will display an error message, indicating that the OS cannot be loaded.

---

## 4. **Operating System Kernel Loading**
After the boot loader is successfully executed, the next step is loading the operating system **kernel**. The kernel is the core part of the OS responsible for managing hardware resources, memory, and process execution.

### Key Functions of the Kernel:
- **Hardware Abstraction**: The kernel abstracts the complexities of interacting with hardware devices, allowing applications to use them without knowing how they work internally.
- **Memory Management**: It initializes the memory and starts organizing it for use by the system and applications.
- **Process Management**: The kernel begins scheduling processes, allocating CPU time to programs and background services.

At this stage, the kernel also sets up drivers for hardware devices like the hard drive, keyboard, mouse, and display, enabling the system to communicate with them.

---

## 5. **Initializing System Processes and Services**
Once the kernel is loaded, it hands over control to the **init system** (on Linux-based systems) or the **Session Manager Subsystem (smss.exe)** in Windows, which handles the rest of the boot process.

### Key Tasks:
- **Background Services**: The init system starts essential background services, such as networking, logging, and printing services.
- **Device Management**: Hardware devices are fully initialized, with their drivers loaded, allowing interaction with peripheral devices like printers, webcams, and external drives.
- **User Authentication**: At this point, the system may display a login screen where users can enter their credentials. The login screen interface is managed by a display manager (such as LightDM or GDM on Linux, and Winlogon on Windows).

---

## 6. **User Space Initialization**
Once the user logs in, the operating system begins setting up the **user space**, which includes the user interface, desktop environment, and any applications that are configured to run at startup.

### Key Events:
- **Graphical User Interface (GUI)**: If the OS uses a graphical interface, the window manager and desktop environment are loaded, providing the user with icons, menus, and other elements of the desktop.
- **Startup Applications**: Applications that are set to run at startup, such as messaging clients, system monitors, or antivirus programs, begin launching.
- **User Profiles**: The system loads the user profile, which includes personal settings like desktop background, preferred themes, and configuration files for specific applications.

At this point, the computer is fully operational, and the user can begin interacting with the system.

---

## Summary of the Boot Process

1. **Power on**: The PSU delivers power and checks voltage levels.
2. **BIOS/UEFI**: The firmware performs hardware checks (POST) and initializes hardware devices.
3. **Boot Loader**: The BIOS/UEFI loads the boot loader from the selected boot device.
4. **Kernel**: The boot loader loads the operating system kernel, which initializes memory, devices, and the CPU.
5. **System Services**: Background services, device drivers, and processes are started by the OS.
6. **User Space**: The desktop environment is loaded, and startup applications launch, allowing the user to log in and begin using the system.

---

## Conclusion

The process that occurs when you power on a computer is intricate and involves multiple layers of hardware and software working together to ensure the system is operational. Each step plays a crucial role in initializing the components and loading the operating system, ensuring the user has a smooth and functional computing experience. From voltage checks to the final login screen, the entire boot process happens in just a few seconds, thanks to years of technological innovation in both hardware and software development.



*© 2024 KeblaOS Project. All rights reserved.*