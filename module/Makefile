
HOST_HOME = /home/bapon

FILE_NAME = user_main


# ====== Compiler & Linker ======
GCC = $(HOST_HOME)/opt/cross/bin/x86_64-elf-gcc-14.2.0
GCC_FLAG = -g -Wall -Wextra -std=gnu11 -m64 \
           -fno-pie -no-pie -nostdlib -nostartfiles \
           -ffreestanding -O2

LD = $(HOST_HOME)/opt/cross/bin/x86_64-elf-ld
LD_FLAG = -m elf_x86_64 -nostdlib -static -z max-page-size=0x1000
LINKER_FILE = user_linker_x86_64.ld

# ====== Directories ======
BUILD_DIR = build

# ====== Source and Object Files ======
SRC_FILES := $(shell find . -name '*.c')
OBJ_FILES := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# ====== Rules ======

# Compile .c to build/%.o
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCC_FLAG) -c $< -o $@

# Linking
linking: $(BUILD_DIR)/$(FILE_NAME).elf

$(BUILD_DIR)/$(FILE_NAME).elf: $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LD_FLAG) -T $(LINKER_FILE) -o $@ $^
	@echo "Linked object files into $@"

	make clean

# Clean
clean:
	# Delete all .o and .d files recursively inside build directory
	find $(BUILD_DIR) -type f \( -name '*.o' -o -name '*.d' \) -delete

	# Remove empty directories (which had only .o, .d files) inside build directory
	find $(BUILD_DIR) -type d -empty -delete

	# Remove all files from build directory
	# rm -rf $(BUILD_DIR)/*

	@echo "Cleaned object files and empty directories."




