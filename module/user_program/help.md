# To Load ELF file by Limine

1. Add the elf file path in `limine.conf` by 

```
    MODULE_PATH: boot():/boot/user_program_1.elf
```

2. Building `elf` file from c file

```
$(GCC) $(GCC_FLAG) -c $(user_cfile).c -o $(user_cfile).o
	$(LD) $(LD_FLAG) -T $(LINKER_FILE) $(user_cfile).o --oformat=elf64-x86-64 -o $(user_cfile).elf
```

3. Copy the `elf file` into boot directory by

```
cp -v $(MODULE_DIR)/user_program_1.elf $(ISO_DIR)/boot/user_program_1.elf
```

4. The Userspace starts from lower half kernel memory space `0x401000` which is defined in linker script.

5. The userspace function is using system call to use kernel.

