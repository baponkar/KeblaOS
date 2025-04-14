# Disk Image

*Created: 14<sup>th</sup> April 2025*

## Introduction

A `disk.img` is a **disk image file** — a complete byte-for-byte copy of a physical disk (like a hard drive, SSD, USB stick, or CD/DVD). It acts as a **virtual disk**, storing everything the physical disk would: partitions, boot records, filesystems, and data files.

### 🔧 Key Properties:
- It's usually created using tools like `dd`, `qemu-img`, or disk utilities.
- It can contain **bootable operating systems**, **file structures**, or even **custom partition schemes**.
- Can be mounted as a loop device on Linux, or opened using tools like OSFMount on Windows.

### 🛠 Common Uses:
- **Operating System Development**: Used to test or install OS builds (like you're doing in KeblaOS).
- **Virtual Machines**: Used by QEMU, VirtualBox, etc., as a virtual hard disk.
- **Disk Cloning & Backup**: Used to back up or restore exact disk states.
- **Forensics & Analysis**: Used to analyze disks without touching the original.

## Overview

A **disk image** (`disk.img`) is a file that emulates a physical disk, containing all the data, partition tables, boot sectors, and filesystem metadata. Disk images are widely used for emulation, virtualization, OS development, backup, and software testing.

In this project, I am creating a **`disk.img`** inside the `Disk` directory, which will serve as a virtual disk drive for **QEMU** during the development and testing of the **KeblaOS** operating system.

The Makefile includes a command:

```bash
make build_disk
```

This command creates a 512 MB FAT32-formatted disk image and mounts it on the `Disk/mnt` directory.

This note demonstrates various use cases and steps related to working with `disk.img`.

---

## 📦 1. Creating a Blank Disk Image

We create a blank disk image of 512MB filled with zeros:

```bash
dd if=/dev/zero of=Disk/disk.img bs=1M count=512
```

---

## 🧱 2. Partitioning the Disk with MBR (Master Boot Record)

To add a partition table and create a FAT32 partition:

```bash
parted Disk/disk.img --script -- mklabel msdos
parted Disk/disk.img --script -- mkpart primary fat32 1MiB 100%
```

The `1MiB` offset is to align the partition properly for performance and compatibility.

---

## 🔍 3. Check if `disk.img` Is Mounted

```bash
mount | grep disk.img
```

This command helps you verify if the image is already mounted.

---

## 🔄 4. Check Loop Device Associations

```bash
losetup -l
```

Output might look like:

```bash
NAME       SIZELIMIT OFFSET AUTOCLEAR RO BACK-FILE
/dev/loop0         0      0         0  0 /path/to/KeblaOS/Disk/disk.img
```

---

## 📌 5. Check Mounted Partition from Image

```bash
lsblk
```

Output (simplified):

```bash
loop0       7:0    0   512M  0 loop
️└─loop0p1 259:0    0   511M  0 part /mnt/disk
```

This tells us `/dev/loop0p1` is the partition we want to mount.

---

## 🕵️ 6. Check Which Processes Are Using `disk.img`

```bash
sudo lsof | grep disk.img
```

Helpful when `umount` fails due to active use.

---

## ❌ 7. Unmount Disk Image (If Mounted)

```bash
sudo umount Disk/mnt || true
```

Use `|| true` to avoid script failure if it's already unmounted.

---

## 🔓 8. Detach the Loop Device

```bash
sudo losetup -d /dev/loop0 || true
```

This releases the loop device from the image file.

---

## 🛠️ 9. Mounting the Partition from `disk.img`

After partitioning, attach and mount it:

```bash
sudo losetup -fP Disk/disk.img
sudo mount /dev/loop0p1 Disk/mnt
```

---

## 📁 10. Browsing Contents Inside `disk.img`

Once mounted, you can directly browse or manipulate files inside `Disk/mnt` just like a normal filesystem.

---

## 🥪 11. Using `disk.img` with QEMU

You can use this image as a virtual hard disk:

```bash
qemu-system-x86_64 -drive file=Disk/disk.img,format=raw
```

---

## ✅ Summary

Disk images are essential for OS development. They simulate real disk environments, allowing for:

- Bootloader installation
- Filesystem formatting
- Kernel deployment
- Testing OS features without needing physical hardware

With tools like `dd`, `parted`, `losetup`, and `mount`, `disk.img` becomes a powerful part of your system programming toolkit.

----------------------------------------------
*© 2025 KeblaOS Project. All rights reserved.*
