# XenevaOS KernelAA64 Makefile (GCC Build)

TARGET = KernelAA64.elf
CROSS_COMPILE ?= aarch64-linux-gnu-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

# Target Board selection (Default: qemu_virt)
BOARD ?= qemu_virt
ifeq ($(BOARD), rpi3)
	BOARD_FLAG = -D__TARGET_BOARD_RPI3__
else
	BOARD_FLAG = -D__TARGET_BOARD_QEMU_VIRT__
endif

INCLUDES = -I../BaseHdr -I../Acpica/include -I../Acpica
CFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
         -O2 \
         -DARCH_ARM64 $(BOARD_FLAG) -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS \
         $(INCLUDES)

OBJDIR = obj

# Find all C and Assembly files recursively
C_SRCS_ALL = $(shell find . -name '*.c')
C_SRCS = $(filter-out ./Board/Rpi3bp/vc4dsi.c, $(C_SRCS_ALL))
S_SRCS = $(shell find . -name '*.s')

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))

OBJS = $(C_OBJS) $(S_OBJS)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking KernelAA64..."
	$(LD) -nostdlib -T linker.ld $(OBJS) -o $@
	@echo "Converting ELF to PE32+..."
	$(CROSS_COMPILE)objcopy -O pei-aarch64-little $@ KernelAA64.exe

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
