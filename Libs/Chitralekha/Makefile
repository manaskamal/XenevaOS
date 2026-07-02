# XenevaOS Chitralekha Library Makefile (GCC Build)

TARGET = libChitralekha.a
CROSS_COMPILE ?= aarch64-linux-gnu-

CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

INCLUDES = -I./ -I../XEClib/includes -I../../BaseHdr -I../XEClib/includes/c++ -I../../Ports/freetype2/include
CXXFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-strict-aliasing \
           -O2 -fPIE -fno-exceptions -fno-rtti \
           -DARCH_ARM64 -D__TARGET_BOARD_QEMU_VIRT__ -D_USE_DLMALLOC -D__STDC_LIMIT_MACROS -DTHEME_DEFAULT -DCOLOR_RGBA \
           $(INCLUDES)

OBJDIR = obj

# Source files for ARM64
CPP_SRCS = audio/audio.cpp \
           boxblur.cpp \
           chitralekha.cpp \
           color.cpp \
           draw.cpp \
           font.cpp \
           keycode.cpp \
           ttf.cpp \
           widgets/base.cpp \
           widgets/button.cpp \
           widgets/defaultmenu.cpp \
           widgets/defaultonoff.cpp \
           widgets/defaultprogressbar.cpp \
           widgets/defaultscrollbar.cpp \
           widgets/defaultslider.cpp \
           widgets/defaulttextbox.cpp \
           widgets/defaultviews.cpp \
           widgets/defaultwindow.cpp \
           widgets/gridview.cpp \
           widgets/icon.cpp \
           widgets/list.cpp \
           widgets/listview.cpp \
           widgets/menubar.cpp \
           widgets/menu.cpp \
           widgets/msgbox.cpp \
           widgets/onoff.cpp \
           widgets/popupwin.cpp \
           widgets/progress.cpp \
           widgets/scrollbar.cpp \
           widgets/scrollpane.cpp \
           widgets/sidebar.cpp \
           widgets/slider.cpp \
           widgets/textbox.cpp \
           widgets/toolbar.cpp \
           widgets/view.cpp \
           widgets/window.cpp

S_SRCS = _fastcpy_arm64.s

CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))
S_OBJS = $(patsubst %.s,$(OBJDIR)/%.o,$(S_SRCS))

OBJS = $(CPP_OBJS) $(S_OBJS)

.PHONY: all clean

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
