[![Badge](https://img.shields.io/badge/FAT32-FS-blue)](https://github.com/baponkar/fat32_fs)


![FAT32 FILESYSTEM IMAGE](./img/social_image.png)

# FAT32 Filesystem

## Description
---
This repository will help to create FAT32 Filesystem and test that filesystem in raw disk image.

This repo has following support

✅ GPT Partition Support

✅ 8.3 File/Directory Name Support

✅ Long File/Directory Name support

✅ Successfull test functions

Here disk inpot/output is using following two functions

```
bool disk_read(uint64_t lba, uint32_t count, void* buffer);
bool disk_write(uint64_t lba, uint32_t count, const void* buffer)
```

To build the project

```
make -B
```

To make a raw disk image
```
make disk
```

To test
```
make test
```

Output should be:
```bash
Created partition 0 on drive 0: Start LBA: 2048, Sectors: 204800
Successfully created Partition at Sector 2048
Created partition 1 on drive 0: Start LBA: 206848, Sectors: 1888256
Successfully created Partition at Sector 206848
Creating FAT32 Volume at LBA 2048 with 204800 sectors
  Initializing FAT 1... Done
  Initializing FAT 2... Done
[FAT32 TEST] Successfully created FAT32 volume at LBA: 2048 with size: 204800 MB
Creating FAT32 Volume at LBA 206848 with 1888256 sectors
  Initializing FAT 1... Done
  Initializing FAT 2... Done
[FAT32 TEST] Successfully created FAT32 volume at LBA: 2048 with size: 1888256 MB
FAT32 mounted
 Volume starts at LBA: 2048
 Bytes/sector: 512
 Sectors/cluster: 2
 Reserved sectors: 32
 FAT size: 794
 Root cluster: 2
 Total clusters: 101590
[FAT32 TEST] Successfully Mount Disk.
[FAT32 TEST] Creating Directory mylongtestdir is success.
[FAT32 TEST] Successfully get Cluster no 3 for directory mylongtestdir
[FAT32 TEST] Successfully created mylongtestfile.text

[FAT32 TEST] Successfully open file mylongtestdir/mylongtestfile.text
[FAT32 TEST] Successfully read 56 bytes
[FAT32 TEST] File content: This is a test text string for testing fat32 filesystem.��
FAT32 test passed successfully!
```

Check the Disk and test Disk Content:

```bash
# Creating a Blank Disk Image of size 1024 bytes i.e. 1 GB
dd if=/dev/zero of=disk_img/disk.img bs=1M count=1024

# Check GPT Partition
gdisk -l disk_img/disk.img

# Creating a loop device for disk.img on first available loopdevice
sudo losetup -fP --show disk_img/disk.img 

# if giving output
/dev/loop1

# Check FAT Partition 
sudo fsck.fat -v -n /dev/loop0p1

# Mount first partition of loop device on mnt dir
sudo mount /dev/loop0p1 disk_img/mnt 

# List root mnt directory
ls -R disk_img/mnt

# Check mylongtestfile.text content
cat disk_img/mnt/mylongtestdir/mylongtestfile.text

# More Debug functions

# Check MBR LBA0
hexdump -C -n 512 disk_img/disk_1.img

# Check GPT Header LAA1
hexdump -C -s 512 -n 92 disk_img/disk_1.img

# Check GPT Partition Table LBA2
hexdump -C -s 1024 -n 512 disk_img/disk_1.img

# Unmount the disk.img
sudo umount /mnt

# Remove specific loop device
sudo losetup -d /dev/loop0

# Remove all loop device
sudo losetup -D

```


© 2026 baponkar. All rights reserved.



