# XenevaOS KernelAA64 Makefile

TARGET = KernelAA64.elf
OBJDIR = obj

# Default toolchain is GCC
TOOLCHAIN ?= gcc

# Triggered when you run "make llvm"
.PHONY: all clean llvm
llvm:
	@$(MAKE) TOOLCHAIN=llvm all

# Target Board selection (Default: qemu_virt)
BOARD ?= qemu_virt
ifeq ($(BOARD), rpi3)
    BOARD_FLAG = -D__TARGET_BOARD_RPI3__
else
    BOARD_FLAG = -D__TARGET_BOARD_QEMU_VIRT__
endif

INCLUDES = -I../BaseHdr -I../Acpica/include -I../Acpica -I../DCL/includes -I.


ifeq ($(TOOLCHAIN), llvm)
    # llvm configuration
    CC      = clang
    LD      = ld.lld
    OBJCOPY = llvm-objcopy
    
    LLVM_TARGET = -target aarch64-unknown-windows
    
    CFLAGS  = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
              -O2 $(LLVM_TARGET) -fno-builtin \
              -DARCH_ARM64 $(BOARD_FLAG) -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS \
              $(INCLUDES)

    LDFLAGS = $(LLVM_TARGET) -fuse-ld=lld -nostdlib \
              -Wl,-entry:_AuMain \
              -Wl,-base:0xFFFFFF8000000000 \
              -Wl,-subsystem:native

else
    # original gcc configuration
    CROSS_COMPILE ?= aarch64-linux-gnu-
    CC      = $(CROSS_COMPILE)gcc
    LD      = $(CROSS_COMPILE)ld
    OBJCOPY = $(CROSS_COMPILE)objcopy
    
    CFLAGS  = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
              -O2 \
              -DARCH_ARM64 $(BOARD_FLAG) -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS \
              $(INCLUDES)
endif


C_SRCS_ALL = $(shell find . -name '*.c')
C_SRCS = $(filter-out ./Board/Rpi3bp/vc4dsi.c, $(C_SRCS_ALL))
S_SRCS = $(shell find . -name '*.s')

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))

OBJS = $(C_OBJS) $(S_OBJS)


all: $(TARGET)

ifeq ($(TOOLCHAIN), llvm)

$(TARGET): $(OBJS)
	@echo "Linking native PE32+ Kernel using LLVM..."
	$(CC) $(LDFLAGS) $(OBJS) -o KernelAA64.exe
	@touch $(TARGET)
else

$(TARGET): $(OBJS)
	@echo "Linking KernelAA64 using GCC..."
	$(LD) -nostdlib -T linker.ld $(OBJS) -o $@
	@echo "Converting ELF to PE32+..."
	$(OBJCOPY) -O pei-aarch64-little $@ KernelAA64.exe
endif

# Compile C source files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM source files
$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) KernelAA64.exe