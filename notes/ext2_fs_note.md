# EXT2 FileSystem Revision 0

🗂️ EXT2 Filesystem Layout (Partition starts at LBA 2048)
```
Disk Layout:
+----------------------------+  LBA 0        (Offset: 0x000000)
| Master Boot Record (MBR)   |
| - Boot Code                |
| - Disk Signature           |
| - Partition Table (4x16B)  |
| - Boot Signature (0xAA55)  |
+----------------------------+  LBA 1 - 2047 (usually empty or reserved)

Partition 1 (EXT2) starts here:
+----------------------------+  LBA 2048     (Offset: 1,048,576 / 0x100000)
| Unused Bootloader Space    |
| (1024 bytes reserved)      |
+----------------------------+  LBA 2050     (Offset: 1,049,600 / 0x100400)
| Superblock (1024 bytes)    |
+----------------------------+  LBA 2052     (Offset: 1,050,624 / 0x100800)
| Group Descriptor Table     |  (usually 1024 bytes if block size = 1 KiB)
+----------------------------+  LBA 2054+    (offset continues)
| Block Bitmap               |
+----------------------------+
| Inode Bitmap               |
+----------------------------+
| Inode Table                |
+----------------------------+
| Data Blocks                |
+----------------------------+ LBA 
| Block Group 1              |
| (with copy of superblock)  |
| and same layout as above   |
+----------------------------+
| ...                        |
+----------------------------+
| Block Group N-1            |
+----------------------------+

```

📦 EXT2 Superblock Structure (Revision 0/1, 1024 bytes)

```
Offset | Size | Field                      | Description
-------|------|----------------------------|----------------------------------------------
0x00   | 4    | s_inodes_count             | Total number of inodes
0x04   | 4    | s_blocks_count             | Total number of blocks
0x08   | 4    | s_r_blocks_count           | Reserved blocks count
0x0C   | 4    | s_free_blocks_count        | Free blocks count
0x10   | 4    | s_free_inodes_count        | Free inodes count
0x14   | 4    | s_first_data_block         | First Data Block (0 for 1KiB block size)
0x18   | 4    | s_log_block_size           | Block size = 1024 << s_log_block_size
0x1C   | 4    | s_log_frag_size            | Fragment size = 1024 << s_log_frag_size
0x20   | 4    | s_blocks_per_group         | Blocks per group
0x24   | 4    | s_frags_per_group          | Fragments per group
0x28   | 4    | s_inodes_per_group         | Inodes per group
0x2C   | 4    | s_mtime                    | Last mount time (UNIX timestamp)
0x30   | 4    | s_wtime                    | Last write time
0x34   | 2    | s_mnt_count                | Mount count since last fsck
0x36   | 2    | s_max_mnt_count            | Max mount count before fsck
0x38   | 2    | s_magic                    | Magic signature (should be 0xEF53)
0x3A   | 2    | s_state                    | Filesystem state (clean, errors, etc.)
0x3C   | 2    | s_errors                   | Behavior when detecting errors
0x3E   | 2    | s_minor_rev_level          | Minor revision level
0x40   | 4    | s_lastcheck                | Time of last fsck
0x44   | 4    | s_checkinterval            | Max time between checks
0x48   | 4    | s_creator_os               | OS that created the filesystem
0x4C   | 4    | s_rev_level                | Revision level (0 = original)
0x50   | 2    | s_def_resuid               | Default UID for reserved blocks
0x52   | 2    | s_def_resgid               | Default GID for reserved blocks

// EXT2 Revision 1 and higher (Dynamic)
0x54   | 4    | s_first_ino                | First non-reserved inode
0x58   | 2    | s_inode_size               | Size of each inode structure
0x5A   | 2    | s_block_group_nr           | Block group of this superblock
0x5C   | 4    | s_feature_compat           | Compatible features
0x60   | 4    | s_feature_incompat         | Incompatible features
0x64   | 4    | s_feature_ro_compat        | Read-only compatible features
0x68   | 16   | s_uuid                     | Filesystem UUID
0x78   | 16   | s_volume_name              | Volume name (null-terminated)
0x88   | 64   | s_last_mounted             | Path last mounted to (null-terminated)
0xC8   | 4    | s_algo_bitmap              | Compression algorithm bitmap

// Padding & Reserved (up to 1024 bytes)

```


📦 Inode Structure (from the Inode Table)
```
+-------------------------------------------------------+
|                  Inode Structure                      |
+-------------------------------------------------------+
| Mode (file type, permissions)                         |
| UID, GID                                              |
| Size of file                                          |
| Timestamps (creation, modification, access)           |
| Link count                                            |
+-------------------------------------------------------+
| Direct Pointers (0-11) -> Data Blocks                 |
+-------------------------------------------------------+
| Singly Indirect Pointer (12) -> Indirect Block        |
|   (Indirect Block -> Data Blocks)                     |
+-------------------------------------------------------+
| Doubly Indirect Pointer (13) -> Doubly Indirect Block |
|   (Doubly Indirect Block -> Singly Indirect Blocks)   |
|   (Singly Indirect Blocks -> Data Blocks)             |
+-------------------------------------------------------+
| Triply Indirect Pointer (14) -> Triply Indirect Block |
|   (Triply Indirect Block -> Doubly Indirect Blocks)   |
|   (Doubly Indirect Blocks -> Singly Indirect Blocks)  |
|   (Singly Indirect Blocks -> Data Blocks)             |
+-------------------------------------------------------+
```

📁 Directory Entry Structure
```
+---------------------------------------+
|          Directory Data Block         |
+---------------------------------------+
|  Directory Entry 1:                   |
|    - Inode Number                     |
|    - Record Length                    |
|    - Name Length                      |
|    - File Type                        |
|    - Filename                         |
+---------------------------------------+
|  Directory Entry 2:                   |
|    - Inode Number                     |
|    - Record Length                    |
|    - Name Length                      |
|    - File Type                        |
|    - Filename                         |
+---------------------------------------+
|                 ...                   |
+---------------------------------------+
```









