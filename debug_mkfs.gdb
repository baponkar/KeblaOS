# Load kernel symbols
file kernel.elf

# Connect to QEMU if remote debugging
target remote :1234

# Set breakpoints at key locations
break vfs_mkfs
break fatfs_mkfs

# Commands to run when vfs_mkfs is called
commands 1
  printf "=== vfs_mkfs called ===\n"
  printf "pd = %d, logical_drive = %d, fs_type = %d\n", pd, logical_drive, fs_type
  printf "disk_count = %d\n", disk_count
  printf "disks pointer = %p\n", disks
  
  # Check if disks[pd] is valid
  if (pd < disk_count && disks != 0)
    printf "disks[%d] = %p\n", pd, &disks[pd]
    printf "disk.type = %d\n", disks[pd].type
  end
  
  # Continue to see where it crashes
  continue
end

# Commands for fatfs_mkfs
commands 2
  printf "=== fatfs_mkfs called ===\n"
  printf "ld = %d, fs_type = %d\n", ld, fs_type
  printf "work buffer allocated at %p\n", work
  printf "root_path = %s\n", root_path
  continue
end

# Run until crash
continue
