TARGET = i686-elf

TOOLS_DIR	= tools/bin
BUILD_DIR	= build
SRC_DIR	   	= src
ISO_DIR	   	= isodir

CC = $(TOOLS_DIR)/$(TARGET)-gcc
AS = $(TOOLS_DIR)/$(TARGET)-as

OS_NAME	   = gurt
KERNEL_BIN	= $(OS_NAME).bin
OS_ISO		= $(OS_NAME).iso

C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = $(wildcard $(SRC_DIR)/*.s)

C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mgeneral-regs-only
LDFLAGS = -T $(SRC_DIR)/linker.ld -ffreestanding -O2 -nostdlib

all: $(BUILD_DIR)/$(OS_ISO)

$(BUILD_DIR)/$(OS_ISO): $(SRC_DIR)/grub.cfg $(BUILD_DIR)/$(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/$(KERNEL_BIN) $(ISO_DIR)/boot/$(KERNEL_BIN)
	cp $(SRC_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/$(OS_ISO) $(ISO_DIR)

$(BUILD_DIR)/$(KERNEL_BIN): $(OBJECTS) $(SRC_DIR)/linker.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) -lgcc

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

run: all
	qemu-system-i386 -cdrom $(BUILD_DIR)/$(OS_ISO) -serial file:serial.log

# Tells make that these rules don't create actual files
.PHONY: all clean run
