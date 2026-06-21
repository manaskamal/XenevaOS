# Target EFI binary
TARGET_EFI = BOOTAA64.efi
TARGET_SO = BOOTAA64.so

# Setting OBJCOPY
OBJCOPY = aarch64-linux-gnu-objcopy

# Source files
CPP_SRCS = clib.cpp devicetree.cpp file.cpp mem.cpp paging.cpp pe.cpp physm.cpp uart0.cpp vector.cpp video.cpp xnldr.cpp xnout.cpp
ASM_SRCS = lowlevel.s vectoraa64.s

# Object directory
OBJDIR = obj

# GNU-EFI Source Directory (Override via env variable or make argument)
GNU_EFI_DIR ?= ../gnu-efi

# Include directories
INCLUDES = -I../BaseHdr -I../Boot/include -I../Boot/include/AArch64 -I$(GNU_EFI_DIR)/inc -I$(GNU_EFI_DIR)/inc/aarch64

# Compiler flags
CXXFLAGS = -Wno-error=pragmas -fPIE \
		   -Wall -Wextra -funsigned-char -fshort-wchar \
		   -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check \
		   -fno-merge-all-constants -std=c++17 -DGNU_EFI_USE_MS_ABI -D__TARGET_BOARD_RPI3__ $(INCLUDES)

# GNU-EFI link setup
LDSCRIPT = ${GNU_EFI_DIR}/gnuefi/elf_aarch64_efi.lds
CRT0 = ${GNU_EFI_DIR}/aarch64/gnuefi/crt0-efi-aarch64.o
LIBS = -lgnuefi -lefi
LIBDIRS = -L${GNU_EFI_DIR}/aarch64/gnuefi -L${GNU_EFI_DIR}/aarch64/lib

# Tools
CXX = aarch64-linux-gnu-g++
CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld

# Output paths
SO_OUTPUT  = $(TARGET_SO)
EFI_OUTPUT = Build/EFI/BOOT/$(TARGET_EFI)

# Object files
CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))
ASM_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(ASM_SRCS))
OBJS = $(CPP_OBJS) $(ASM_OBJS)

.PHONY: all clean

all: $(EFI_OUTPUT)

# Link aarch64 EFI binary
$(SO_OUTPUT): $(CRT0) $(OBJS)
	$(LD) -nostdlib --warn-common --no-undefined --build-id=sha1 -z noexecstack \
	-shared -Bsymbolic $(LIBDIRS) -T$(LDSCRIPT) $(CRT0) $(OBJS) -o $@ $(LIBS)

$(EFI_OUTPUT): $(SO_OUTPUT)
	@mkdir -p $(dir $@)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela \
			   -j .rel.* -j .rela.* \
	           -j .reloc --target=efi-app-aarch64 --subsystem=10 $< $@

# Compile C++ source files
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Assemble ASM source files
$(OBJDIR)/%.o: %.s
	@mkdir -p $(OBJDIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) Build $(SO_OUTPUT) $(EFI_OUTPUT)
