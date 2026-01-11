# OS Detection
ifeq ($(OS),Windows_NT)
    RM = rmdir /s /q
    MKDIR = mkdir
else
    RM = rm -rf
    MKDIR = mkdir -p
endif

# Toolchain setup
ASM = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

# Flags
ASMFLAGS = -f bin
CFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld

# Directories
SRC_DIR = src
BUILD_DIR = build
TOOLS_DIR = tools

# Targets
KERNEL_SRC = $(SRC_DIR)/kernel/kernel.c
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
DRIVERS_SRC = $(wildcard $(SRC_DIR)/drivers/*.c)
DRIVERS_OBJ = $(patsubst $(SRC_DIR)/drivers/%.c, $(BUILD_DIR)/%.o, $(DRIVERS_SRC))
LIBC_SRC = $(wildcard $(SRC_DIR)/libc/*.c)
LIBC_OBJ = $(patsubst $(SRC_DIR)/libc/%.c, $(BUILD_DIR)/%.o, $(LIBC_SRC))
CPU_SRC = $(wildcard $(SRC_DIR)/cpu/*.c)
CPU_OBJ = $(patsubst $(SRC_DIR)/cpu/%.c, $(BUILD_DIR)/%.o, $(CPU_SRC))

KERNEL_ENTRY_SRC = $(SRC_DIR)/boot/kernel_entry.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
MBR_SRC = $(SRC_DIR)/boot/mbr.asm
MBR_BIN = $(BUILD_DIR)/mbr.bin
STAGE2_SRC = $(SRC_DIR)/boot/stage2.asm
STAGE2_BIN = $(BUILD_DIR)/stage2.bin
OS_IMAGE = $(BUILD_DIR)/os-image.bin

all: $(OS_IMAGE)

run: $(OS_IMAGE)
	qemu-system-i386 -hda $(OS_IMAGE)

$(OS_IMAGE): $(MBR_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	python3 $(TOOLS_DIR)/build_image.py $@ $^

$(MBR_BIN): $(MBR_SRC)
	-$(MKDIR) $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(STAGE2_BIN): $(STAGE2_SRC)
	-$(MKDIR) $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	-$(MKDIR) $(BUILD_DIR)
	$(ASM) -f elf32 $< -o $@

$(KERNEL_BIN): $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) $(DRIVERS_OBJ) $(LIBC_OBJ) $(CPU_OBJ)
	$(LD) -o $@ -Ttext 0x1000 $^ --oformat binary

$(KERNEL_OBJ): $(KERNEL_SRC)
	-$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/drivers/%.c
	-$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/libc/%.c
	-$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/cpu/%.c
	-$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-$(RM) $(BUILD_DIR)

.PHONY: all clean
