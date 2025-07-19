# EXT2 FileSystem Revision 0

```
+------------------------------------+
|  LBA Sector 0 (512 bytes)          |
+------------------------------------+
| Master Boot Code                   |
+------------------------------------+
| Disk Signature                     |
+------------------------------------+
| Partition Table (defines partitions)|
|  - Entry for Partition 1 (e.g., points to start of ext2 partition) |
|  - Entry for Partition 2           |
|  - Entry for Partition 3           |
|  - Entry for Partition 4           |
+------------------------------------+
| End of MBR Marker (0xAA55)         |
+------------------------------------+
|                                    |
|  <<< START OF PARTITION 1 (e.g., where an ext2 filesystem begins) >>> |
|                                    |
+------------------------------------+
|  Offset 0 within Partition 1       |
+------------------------------------+
|  Boot Block (1024 bytes)           |
+------------------------------------+
|  Block Group 0 (of ext2 filesystem) |
|    - Superblock (copy)             |
|    - Group Descriptor Table (copy) |
|    - Data Block Bitmap             |
|    - Inode Bitmap                  |
|    - Inode Table                   |
|    - Data Blocks                   |
+------------------------------------+
|  Block Group 1 (of ext2 filesystem) |
|    - Superblock (copy)             |
|    - Group Descriptor Table (copy) |
|    - Data Block Bitmap             |
|    - Inode Bitmap                  |
|    - Inode Table                   |
|    - Data Blocks                   |
+------------------------------------+
|        ...                         |
+------------------------------------+
|  Block Group N-1 (of ext2 filesystem) |
+------------------------------------+
|                                    |
|  <<< END OF PARTITION 1 >>>        |
|                                    |
+------------------------------------+
|                                    |
|  Other Partitions (Partition 2, 3, etc. as defined by MBR) |
|  +--------------------------------+  |
|  | ...                            |  |
|  +--------------------------------+  |
|                                    |
+------------------------------------+
```


```
+-------------------------------------------------+
|                  Inode Structure                |
+-------------------------------------------------+
| Mode (file type, permissions)                   |
| UID, GID                                        |
| Size of file                                    |
| Timestamps (creation, modification, access)     |
| Link count                                      |
+-------------------------------------------------+
| Direct Pointers (0-11) -> Data Blocks           |
+-------------------------------------------------+
| Singly Indirect Pointer (12) -> Indirect Block  |
|   (Indirect Block -> Data Blocks)               |
+-------------------------------------------------+
| Doubly Indirect Pointer (13) -> Doubly Indirect Block |
|   (Doubly Indirect Block -> Singly Indirect Blocks)  |
|   (Singly Indirect Blocks -> Data Blocks)          |
+-------------------------------------------------+
| Triply Indirect Pointer (14) -> Triply Indirect Block |
|   (Triply Indirect Block -> Doubly Indirect Blocks) |
|   (Doubly Indirect Blocks -> Singly Indirect Blocks)|
|   (Singly Indirect Blocks -> Data Blocks)        |
+-------------------------------------------------+
```

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