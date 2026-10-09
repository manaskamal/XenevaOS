# XenevaOS KernelAA64 Makefile

TARGET = KernelAA64.elf
OBJDIR = obj

# MSYS2 shells (MINGW64/CLANG64/UCRT64/MSYS/...) set $MSYSTEM. There's no
# aarch64-linux-gnu-gcc cross toolchain packaged for MSYS2, so default to
# llvm there; everywhere else (Linux) keeps defaulting to gcc.
ifdef MSYSTEM
    TOOLCHAIN ?= llvm
endif
TOOLCHAIN ?= gcc

BLEED ?= 0
ifeq ($(BLEED),1)
    BLEED_FLAGS = -D__XENEVA_BLEED__
endif

SOAK ?= 0
ifeq ($(SOAK),1)
    SOAK_FLAGS = -D__XENEVA_SOAK__
endif

# Stage 1 always allocates owner accounting; Stages 2-3 (redzones,
# poisoning, caller tracking, leak dump, frame-table owner scan) compile
# in only with DEBUG_ALLOC=1. Threaded through from
# Scripts/Linux/build_and_run_qemu.sh --debug-alloc.
DEBUG_ALLOC ?= 0
ifeq ($(DEBUG_ALLOC),1)
    DEBUG_ALLOC_FLAGS = -D__XENEVA_DEBUG_ALLOC__
endif

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

INCLUDES = -I../BaseHdr -I../Acpica/include -I../Acpica -I.

EXPORTS_DEF = exports.def

ifeq ($(TOOLCHAIN), llvm)
    # llvm configuration
    CC      = clang
    LD      = ld.lld
    OBJCOPY = llvm-objcopy
    
    LLVM_TARGET = -target aarch64-unknown-windows
    
    CFLAGS  = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
              -O2 -g -gdwarf-4 $(LLVM_TARGET) -fno-builtin \
              -DARCH_ARM64 $(BOARD_FLAG) -D__STDC_LIMIT_MACROS $(BLEED_FLAGS) $(SOAK_FLAGS) $(DEBUG_ALLOC_FLAGS) \
              -Wno-error=incompatible-pointer-types -Wno-error=int-conversion \
              -Wno-implicit-function-declaration -Wno-error=conflicting-types $(INCLUDES)

    LDFLAGS = $(LLVM_TARGET) -fuse-ld=lld -nostdlib \
              -Wl,-entry:_AuMain \
            -Wl,-base:0xFFFFC00000000000 \
            -Wl,-subsystem:native \
            -Wl,-def:$(EXPORTS_DEF) \
            -Wl,-map:KernelAA64.map

    ifeq ($(shell command -v $(CC) 2>/dev/null),)
        $(error $(CC) not found on PATH. On MSYS2: pacman -S mingw-w64-clang-x86_64-toolchain (or the clangarm64/ucrt64 package matching your shell), then reopen the shell)
    endif
    ifeq ($(shell command -v ld.lld 2>/dev/null),)
        $(error ld.lld not found on PATH. It ships with the clang toolchain package above)
    endif

else
    # original gcc configuration
    ifdef MSYSTEM
        $(error TOOLCHAIN=gcc needs aarch64-linux-gnu-gcc, which MSYS2 doesn't package. Run "make llvm" instead (MSYS2 already defaults to it))
    endif

    CROSS_COMPILE ?= aarch64-linux-gnu-
    CC      = $(CROSS_COMPILE)gcc
    LD      = $(CROSS_COMPILE)ld
    OBJCOPY = $(CROSS_COMPILE)objcopy

    CFLAGS  = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
              -O2 \
              -DARCH_ARM64 $(BOARD_FLAG) -D__STDC_LIMIT_MACROS $(BLEED_FLAGS) $(SOAK_FLAGS) $(DEBUG_ALLOC_FLAGS) \
              $(INCLUDES)
endif


C_SRCS_ALL = $(shell find . -name '*.c')
# Exclude dead allocator code (replaced by TLSF)
C_SRCS = $(filter-out ./Mm/dlmalloc.c ./Mm/liballoc.c, $(filter-out ./Board/Rpi3bp/vc4dsi.c, $(C_SRCS_ALL)))
S_SRCS = $(shell find . -name '*.s')

# DCL (Driver Compatibility Layer) sources live outside KernelAA64, in ../DCL.
# Their objects are rewritten into $(OBJDIR)/DCL/ so nothing escapes obj/.
DCL_C_SRCS := $(patsubst ../DCL/%,%,$(shell find ../DCL -name '*.c' 2>/dev/null))
DCL_S_SRCS := $(patsubst ../DCL/%,%,$(shell find ../DCL -name '*.s' 2>/dev/null))

# Vendored mainline sources live in ../Vendored: byte-identical copies of
# upstream *.c compiled straight into the image. Same treatment as ../DCL, in
# a separate tree, for two reasons that only show up once a few drivers are
# in here:
#
#   1. ../DCL is enumerated by the test gate ("DCL sources = N"), so moving a
#      driver into it changes an assertion meant to track DCL itself.
#   2. the Makefile enumerates ../DCL with $(shell find ...), so a file
#      dropped in there while a build holds Build/.build.lock would be
#      compiled mid-run. A separate tree keeps that hazard scoped to DCL.
#
# A .c here is expected to be unmodified from upstream: porting fixes go in
# BaseHdr/ or in a ../DCL shim, never in the vendored file. DCL/tty_buffer.c
# is held to the same rule (md5-identical to v7.2) for the same reason.
VENDORED_C_SRCS := $(patsubst ../Vendored/%,%,$(shell find ../Vendored -name '*.c' 2>/dev/null))
VENDORED_S_SRCS := $(patsubst ../Vendored/%,%,$(shell find ../Vendored -name '*.s' 2>/dev/null))

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))
DCL_C_OBJS = $(patsubst %.c,$(OBJDIR)/DCL/%.o,$(DCL_C_SRCS))
DCL_S_OBJS = $(patsubst %.s,$(OBJDIR)/DCL/%.o,$(DCL_S_SRCS))
VENDORED_C_OBJS = $(patsubst %.c,$(OBJDIR)/Vendored/%.o,$(VENDORED_C_SRCS))
VENDORED_S_OBJS = $(patsubst %.s,$(OBJDIR)/Vendored/%.o,$(VENDORED_S_SRCS))

OBJS = $(C_OBJS) $(S_OBJS) $(DCL_C_OBJS) $(DCL_S_OBJS) $(VENDORED_C_OBJS) $(VENDORED_S_OBJS)


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

# Compile DCL (Driver Compatibility Layer) sources from ../DCL
$(OBJDIR)/DCL/%.o: ../DCL/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM source files
$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble DCL ASM source files from ../DCL
$(OBJDIR)/DCL/%.o: ../DCL/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile vendored mainline sources from ../Vendored (same flags as everything
# else -- they see BaseHdr/linux via $(INCLUDES), which is the whole point:
# the headers become the contract the driver is compiled against).
#
# KBUILD_MODNAME mirrors what kbuild passes a module: the driver's own basename,
# used for .driver.name. $(basename $(notdir $<)) -- "virtio-rng" from
# ../Vendored/drivers/char/hw_random/virtio-rng.c. The nested quoting is the
# point: the shell has to deliver -DKBUILD_MODNAME="virtio-rng" as one argv
# entry so clang sees a string literal, not an identifier.
$(OBJDIR)/Vendored/%.o: ../Vendored/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DKBUILD_MODNAME='"$(basename $(notdir $<))"' -c $< -o $@

# Assemble vendored ASM source files from ../Vendored
$(OBJDIR)/Vendored/%.o: ../Vendored/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) KernelAA64.exe
