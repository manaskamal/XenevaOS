# XenevaOS XEClib Makefile (GCC / LLVM Build)

TARGET = libXEClib.a

# Default toolchain is GCC
TOOLCHAIN ?= gcc

# Triggered when you run "make llvm"
.PHONY: all clean llvm
llvm:
	@$(MAKE) TOOLCHAIN=llvm all

INCLUDES = -I./includes -I./includes/c++ -I../../BaseHdr

ifeq ($(TOOLCHAIN), llvm)
	# LLVM / Clang configuration
	CXX = clang++
	CC  = clang
	AR  = llvm-ar

	LLVM_TARGET = -target aarch64-unknown-windows

	CXXFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check \
	           -fno-strict-aliasing -O2 $(LLVM_TARGET) \
	           -DARCH_ARM64 -D__TARGET_BOARD_QEMU_VIRT__ -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS \
	           -Wno-error=incompatible-pointer-types -Wno-error=int-conversion \
	           -Wno-implicit-function-declaration -Wno-c++11-narrowing \
	           $(INCLUDES)
else
	# GCC configuration
	CROSS_COMPILE ?= aarch64-linux-gnu-
	CXX = $(CROSS_COMPILE)g++
	CC  = $(CROSS_COMPILE)gcc
	AR  = $(CROSS_COMPILE)ar

	CXXFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
	           -O2 -fPIE \
	           -DARCH_ARM64 -D__TARGET_BOARD_QEMU_VIRT__ -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS \
	           $(INCLUDES)
endif

OBJDIR = obj

# Source files for ARM64
CPP_SRCS = acosf.cpp arpa/inet.cpp atan2f.cpp atanf.cpp c++/ios.cpp c++/iostream.cpp \
           c++/istream.cpp c++/memory.cpp c++/new.cpp c++/ostream.cpp c++/_string.cpp \
           ceilf.cpp ctype.cpp errno.cpp fmodf.cpp math.cpp qsort.cpp roundf.cpp \
           stat.cpp stdio.cpp stdlib.cpp string.cpp sys/xenet.cpp sys/_heap.cpp \
           sys/_procheap.cpp            sys/_xeneva.cpp sys/_xeprint.cpp time.cpp \
           _xeinput.cpp _xeprint.cpp cxx_abi.cpp

S_SRCS = _callbase_arm64.s setjmp_arm64.s store_stack_arm64.s

CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))

OBJS = $(CPP_OBJS) $(S_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Archiving $(TARGET)..."
	$(AR) rcs $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
