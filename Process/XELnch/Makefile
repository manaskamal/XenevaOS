# XenevaOS XELnch Makefile (GCC Build)

TARGET = xelnch.elf
CROSS_COMPILE ?= aarch64-linux-gnu-

CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

INCLUDES = -I../../Libs/XEClib/includes -I../../Libs/XEClib/includes/c++ -I../../BaseHdr -I../../Libs/Chitralekha -I../../Ports/freetype2/include
CXXFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
           -O2 -fPIE -fno-exceptions -fno-rtti \
           -DARCH_ARM64 -D__TARGET_BOARD_QEMU_VIRT__ -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS  -DTHEME_DEFAULT \
           $(INCLUDES)

OBJDIR = obj

CPP_SRCS = AppGrid.cpp button.cpp lconfig.cpp main.cpp pagebutton.cpp pwbutton.cpp \
           rrect.cpp search.cpp searchbar.cpp
S_SRCS = crt_arm64.s

CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))

OBJS = $(CPP_OBJS) $(S_OBJS)

.PHONY: all clean

all: xelnch.exe

xelnch.exe: $(TARGET)
	@echo "Converting ELF to PE32+ (xelnch.exe)..."
	$(OBJCOPY) -O pei-aarch64-little $(TARGET) xelnch.exe

$(TARGET): $(OBJS)
	@echo "Linking xelnch.elf..."
	$(LD) -nostdlib -T linker.ld $(OBJS) ../../Libs/Chitralekha/libChitralekha.a ../../Libs/XEClib/libXEClib.a -o $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) xelnch.exe
