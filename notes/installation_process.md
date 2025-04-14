

# Operating System Installation Process to Disk Drive

This document outlines the complete steps for installing a custom Operating System onto a disk drive using a bootable USB and Limine bootloader.

---

## 1. Prepare Bootable ISO

- Create an ISO image compatible with optical drives using:
  - `kernel.bin`
  - `limine.conf`
  - Necessary Limine bootloader files (e.g., `limine.sys`, `limine-cd.bin`, `limine-eltorito-efi.bin`)
- Use tools like `xorriso` or `grub-mkrescue` to generate the ISO.

Example command:
```bash
xorriso -as mkisofs -b limine-cd.bin \
  -no-emul-boot -boot-load-size 4 -boot-info-table \
  --efi-boot limine-eltorito-efi.bin \
  -efi-boot-part --efi-boot-image --protective-msdos-label \
  -o bootable.iso iso_root
```

---

## 2. Create Bootable USB

- Use **Rufus** or `dd` (on Linux) to flash the ISO to a USB drive.

---

## 3. Boot from USB

- Insert the USB into the target machine.
- Boot the machine from the USB.
- Limine bootloader will launch and load the kernel.

---

## 4. Bootloader Execution

- Limine loads `kernel.bin` into memory.
- Transfers control to the kernel's entry point.

---

## 5. Kernel Initialization

Once the kernel starts executing, it should initialize:

- **Global Descriptor Table (GDT)**
- **Interrupt Descriptor Table (IDT)**
- **Physical Memory Manager (PMM)**
- **Virtual Memory Manager (VMM)**
- **Paging (if used)**
- **Interrupt Service Routines (ISRs)**
- **Timer and Keyboard drivers**
- **Framebuffers or Text mode for output**

---

## 6. Hardware Detection

- Scan and initialize essential hardware like:
  - PCI/PCIe devices
  - Storage controllers
  - Input devices (keyboard/mouse)
  - ACPI tables if available

---

## 7. Start the OS Installer

- Once the kernel is fully initialized, launch the **Installer** program.

Installer should perform the following:

- Allow disk selection
- Partition the disk (MBR or GPT)
- Format the target partition with a filesystem (e.g., ext2/ext4, FAT32)
- Copy kernel, init files, and bootloader (Limine) to the disk
- Install Limine bootloader to the disk

---

## 8. Shutdown System

- After successful installation, shut down the system.

---

## 9. Remove USB Drive

- Physically disconnect the USB drive from the system.

---

## 10. Boot from Installed Disk

- Power on the system again.
- Now, Limine bootloader runs from the disk drive.
- It loads the kernel from the disk and boots the OS.

---

## ✅ Notes

- Ensure Limine is installed properly on the target disk using `limine-install` if needed.
- Make sure `limine.cfg` and kernel are correctly placed in the root of the partition.
- The partition used for installation should be marked bootable (active) in MBR.

---

## ✅ Optional (Advanced)

- Setup initramfs or early user-space init.
- Add support for logging, filesystem checking (fsck), or recovery mode.


