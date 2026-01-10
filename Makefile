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

# Targets
KERNEL_SRC = $(SRC_DIR)/kernel/kernel.c
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
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
	cat $^ > $@
	dd if=/dev/zero bs=512 count=64 >> $@

$(MBR_BIN): $(MBR_SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(STAGE2_BIN): $(STAGE2_SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf32 $< -o $@

$(KERNEL_BIN): $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) -o $@ -Ttext 0x1000 $^ --oformat binary

$(KERNEL_OBJ): $(KERNEL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
