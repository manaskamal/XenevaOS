# Target EFI binary
.DEFAULT_GOAL := all
TARGET_EFI = BOOTAA64.efi
TARGET_SO = BOOTAA64.so

# Default toolchain is GCC
TOOLCHAIN ?= gcc

# Triggered when you run "make llvm"
.PHONY: all clean llvm
llvm:
	@$(MAKE) TOOLCHAIN=llvm all

# Source files
CPP_SRCS = clib.cpp devicetree.cpp file.cpp mem.cpp paging.cpp pe.cpp physm.cpp uart0.cpp vector.cpp video.cpp xnldr.cpp xnout.cpp
ASM_SRCS = lowlevel.s vectoraa64.s

# Object directory
OBJDIR = obj

# GNU-EFI Source Directory (Override via env variable or make argument)
GNU_EFI_DIR ?= ../gnu-efi

# Include directories
INCLUDES = -I../BaseHdr -I../Boot/include -I../Boot/include/AArch64 -I$(GNU_EFI_DIR)/inc -I$(GNU_EFI_DIR)/inc/aarch64

# Target Board selection (Default: qemu_virt)
BOARD ?= qemu_virt
ifeq ($(BOARD), rpi3)
	BOARD_FLAG = -D__TARGET_BOARD_RPI3__
else
	BOARD_FLAG = -D__TARGET_BOARD_QEMU_VIRT__
endif

# Output paths
SO_OUTPUT  = $(TARGET_SO)
EFI_OUTPUT = Build/EFI/BOOT/$(TARGET_EFI)

# Object files
CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))
ASM_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(ASM_SRCS))
OBJS = $(CPP_OBJS) $(ASM_OBJS)

ifeq ($(TOOLCHAIN),llvm)

#*	llvm configuration
	CC		=clang
	CXX		=clang++

	LLVM_TARGET = -target aarch64-unknown-windows

	CXXFLAGS = $(LLVM_TARGET) -Wno-error=pragmas \
               -Wall -Wextra -funsigned-char -fshort-wchar \
               -ffreestanding -fno-stack-protector \
               -std=c++17 $(BOARD_FLAG) $(INCLUDES)

	LDFLAGS  = $(LLVM_TARGET) -fuse-ld=lld -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application

else

#* original gcc configuration
	CC      = aarch64-linux-gnu-gcc
    CXX     = aarch64-linux-gnu-g++
    LD      = aarch64-linux-gnu-ld
    OBJCOPY = aarch64-linux-gnu-objcopy

    CXXFLAGS = -Wno-error=pragmas -fPIE \
               -Wall -Wextra -funsigned-char -fshort-wchar \
               -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check \
               -fno-merge-all-constants -std=c++17 -DGNU_EFI_USE_MS_ABI $(BOARD_FLAG) $(INCLUDES)

    LDSCRIPT = ${GNU_EFI_DIR}/gnuefi/elf_aarch64_efi.lds
    CRT0     = ${GNU_EFI_DIR}/aarch64/gnuefi/crt0-efi-aarch64.o
    LIBS     = -lgnuefi -lefi
    LIBDIRS  = -L${GNU_EFI_DIR}/aarch64/gnuefi -L${GNU_EFI_DIR}/aarch64/lib
endif

# build rules
all: $(EFI_OUTPUT)

ifeq ($(TOOLCHAIN), llvm)
$(EFI_OUTPUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@
else
# Original GCC compilation
$(SO_OUTPUT): $(CRT0) $(OBJS)
	$(LD) -nostdlib --warn-common --no-undefined --build-id=sha1 -z muldefs -z noexecstack -z max-page-size=4096 \
	-shared -Bsymbolic $(LIBDIRS) -T$(LDSCRIPT) $(CRT0) $(OBJS) -o $@ $(LIBS)

$(EFI_OUTPUT): $(SO_OUTPUT)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel -j .rela \
	           -j .rel.* -j .rela.* -j .rel* -j .rela* -j .areloc \
	           -j .reloc --target=efi-app-aarch64 --subsystem=10 $< $@
endif

# Compile C++ source files
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Assemble ASM source files
$(OBJDIR)/%.o: %.s
	@mkdir -p $(OBJDIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) Build $(TARGET_SO)