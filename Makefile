# Description: Makefile for the OS
# Author: Will Hopkins

KERNEL_BIN  ?= bin/kernel.bin
BOOT_BIN    ?= bin/boot.bin
OS_BIN      ?= bin/os.bin

LD_SCRIPT   ?= src/linker.ld

AS          = nasm
CC          = i686-elf-gcc
LD		    = i686-elf-ld
EMU		    = qemu-system-i386#qemu-system-x86_64

CC_FLAGS    ?= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
EMU_FLAGS   ?= -hda

SRC_DIR     ?= ./src
BUILD_DIR   ?= ./build
INCLUDE_DIR ?= ./include/

SRC_DIRS 	:= $(shell find $(SRC_DIR) -type d ! -path '*boot' | sed 's|^$(SRC_DIR)||')
BUILD_DIRS 	:= $(addprefix build,$(SRC_DIRS))

SRC_FILES   := $(shell find $(SRC_DIR) -type f ! -name '*.ld' ! -path '*boot/*' | sed 's|^$(SRC_DIR)||')
OBJ_FILES   := $(patsubst %.asm, %.asm.o, $(patsubst %.c, %.o,$(addprefix build,$(SRC_FILES))))
KERNEL_OBJ  := $(BUILD_DIR)/kernelfull.o

# $(addprefix -I,$(shell find $(INCLUDE_DIR) -type d -print))
INC_FLAGS   := -I$(INCLUDE_DIR)

.PHONY: default
default: build

.PHONY: run
run: build
	$(EMU) $(EMU_FLAGS) $(OS_BIN)

build: $(BUILD_DIRS) $(OS_BIN)

$(BUILD_DIRS):
	mkdir -p $(BUILD_DIRS)

$(OS_BIN): $(BUILD_DIRS) $(KERNEL_BIN) $(BOOT_BIN)
	rm -f $(OS_BIN)
	dd if=$(BOOT_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=1048576 count=16 >> $(OS_BIN)
	sudo mount -t vfat ./bin/os.bin /mnt/d
	sudo cp ./hello.txt /mnt/d/
	sudo umount /mnt/d

$(KERNEL_BIN): $(OBJ_FILES)
	$(LD) -g -relocatable $(OBJ_FILES) -o $(KERNEL_OBJ)
	$(CC) $(CC_FLAGS) -T $(LD_SCRIPT) -o $(KERNEL_BIN) -ffreestanding -O0 -nostdlib ./build/kernelfull.o

$(BOOT_BIN): ./src/boot/boot.asm
	$(AS) -f bin $< -o $@

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	$(AS) -f elf $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(INC_FLAGS) $(CC_FLAGS) -std=gnu99 -c $< -o $@

.PHONY: clean-all
clean-all: clean
	rm -f $(OS_BIN)

.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)/**
	rm -f $(KERNEL_BIN)
	rm -f $(BOOT_BIN)
