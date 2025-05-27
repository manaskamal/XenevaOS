/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

#include <stdint.h>

#include <aucon.h>
#include <_null.h>
#include "font.h"
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <Mm\kmalloc.h>
#include <string.h>
#include <va_list.h>
#include <stdio.h>

uint8_t* font_data;
uint32_t console_x;
uint32_t console_y;
uint32_t* __framebuffer;
AuConsole* aucon;
uint32_t redmask, bluemask, greenmask;
uint32_t resvmask;
size_t h_res, v_res;
BOOL early_;


void(*_print_func) (const char* text, ...);

#define CONSOLE_BACKGROUND 0x00000000
#define CONSOLE_FOREGROUND 0xFFFFFFFF

/*
 * AuConsoleInitialize -- initialize kernel direct screen
 * console
 * @param info -- Pointer to kernel boot info structure
 */
void AuConsoleInitialize(PKERNEL_BOOT_INFO info, bool early) {
	if (early) {
		_print_func = info->printf_gui;
		early_ = early;
	}
	aucon = NULL;
}



/*
 * AuConsolePostInitialise -- initialise the post console process
 * @param info -- pointer to kernel boot info structure
 */
void AuConsolePostInitialise(PKERNEL_BOOT_INFO info) {
	aucon = (AuConsole*)kmalloc(sizeof(AuConsole));
	memset(aucon, 0, sizeof(AuConsole));

	for (int i = 0; i < info->fb_size / PAGE_SIZE; i++)
		AuMapPage((uint64_t)info->graphics_framebuffer + (i * PAGE_SIZE),
			0xFFFFD00000200000 +  (i * 4096), PTE_NORMAL_MEM);
	early_ = false;
	aucon->buffer = (uint32_t*)0xFFFFD00000200000;
	aucon->width = info->X_Resolution;
	aucon->height = info->Y_Resolution;
	aucon->bpp = 32;
	aucon->scanline = info->pixels_per_line;
	aucon->pitch = 4 * info->pixels_per_line;
	aucon->size = info->fb_size;
	aucon->early_mode = false;
	console_x = console_y = 0;
	redmask = info->redmask;
	greenmask = info->greenmask;
	bluemask = info->bluemask;
	h_res = info->X_Resolution; v_res = info->Y_Resolution;
	for (int w = 0; w < info->X_Resolution; w++) {
		for (int h = 0; h < info->Y_Resolution; h++) {
			aucon->buffer[w + h * info->X_Resolution] = CONSOLE_BACKGROUND;
		}
	}
	early_ = false;
}


int_fast8_t high_set_bit(size_t sz) {
	int_fast8_t count = -1;
	while (sz) {
		sz >>= 1;
		count += 1;
	}
	return count;
}

int_fast8_t low_set_bit(size_t sz) {
	int_fast8_t count = -1;
	while (sz) {
		count += 1;
		if ((sz & 1) != 0)
			break;
		sz >>= 1;
	}
	return count;
}

#define RGB(r, g, b) \
	         ((r & 0xFF) | ((g << 8)&0xFF00) | ((b << 16)&0xFF0000))

#define RED(col)\
	         (col & 0xFF)

#define GREEN(col)\
	((col>>8) & 0xFF)

#define BLUE(col) \
	((col>>16) & 0xFF)

/*
 * AuPutPixel -- puts a pixel on the screen
 * @param x -- x location of the screen
 * @param y -- y location of the screen
 * @param col -- color of the pixel
 */
void AuPutPixel(size_t x, size_t y, uint32_t col) {

	uint32_t* framebuffer = aucon->buffer;
	uint32_t bpp = aucon->bpp;
	uint32_t pixelPerLine = aucon->scanline;

	uint32_t* pixelloc = RAW_OFFSET(uint32_t*,framebuffer, (pixelPerLine * y + x) * (bpp / 8));
	size_t pixel = ((RED(col) << low_set_bit(redmask)) & redmask) |
		((GREEN(col) << low_set_bit(greenmask)) & greenmask) |
		((BLUE(col) << low_set_bit(bluemask)) & bluemask) |
		(*pixelloc & resvmask);
	*pixelloc = pixel;
}

//! Put a character to console output
void AuPutC(char c) {
	if (console_x > v_res / 9) {
		console_x = 0;
		console_y++;
	}

	for (size_t y = 0; y < 16; ++y) {
		for (size_t x = 0; x < 8; ++x) {
			const bx_fontcharbitmap_t entry = bx_vgafont[c];
			if (entry.data[y] & (1 << x)) {
				AuPutPixel(x + console_x * 9, y + console_y * 16, CONSOLE_FOREGROUND);
			}
			else {
				AuPutPixel(x + console_x * 9, y + console_y * 16, CONSOLE_BACKGROUND);
			}
		}
		AuPutPixel(8 + console_x * 9, y + console_y * 16, CONSOLE_BACKGROUND);
	}

	console_x++;

	uint32_t* lfb = aucon->buffer;
	if (console_y + 1 > h_res / 16)
	{
		for (int i = 16; i < h_res * v_res; i++)
			lfb[i] = lfb[i + v_res * 16];
		console_y--;
	}

}



//! Prints string to console output
void AuPutS(char* str) {
	uint32_t* lfb = aucon->buffer;
	while (*str) {

		if (*str > 0xFF) {
			//unicode
		}
		else if (*str == '\n') {
			++console_y;
			console_x = 0;
		}
		else if (*str == '\r') {
		}
		else if (*str == '\b') {
			if (console_x > 0)
				--console_x;
		}
		else {

			const bx_fontcharbitmap_t entry = bx_vgafont[*str];
			for (size_t y = 0; y < 16; ++y) {

				for (size_t x = 0; x < 8; ++x) {

					if (entry.data[y] & (1 << x)) {
						AuPutPixel(x + console_x * 9, y + console_y * 16, CONSOLE_FOREGROUND);
					}
					else {
						AuPutPixel(x + console_x * 9, y + console_y * 16, CONSOLE_BACKGROUND);
					}
				}
				AuPutPixel(8 + console_x * 9, y + console_y * 16, CONSOLE_BACKGROUND);
			}
			++console_x;
			if (console_x > h_res / 9) {
				console_x = 0;
				++console_y;
			}
		}

		++str;
	}


	/* Scroll */
	if (console_y + 1 > v_res / 16)
	{
		for (int i = 16; i < v_res * h_res; i++)
			lfb[i] = lfb[i + h_res * 16];
		console_y--;
	}
}

/*
 * AuTextOut -- standard text printing function
 * for entire kernel
 * @param text -- text to output
 */
void AuTextOut(const char* format, ...) {
	if (early_) {
		_print_func(format);
		return;
	}

	uint64_t buffer[7];
	store_x0_x7(buffer);

	va_list args = (va_list)buffer;
	while (*format)
	{
		if (*format == '%')
		{
			++format;
			if (*format == 'd')
			{
				size_t width = 0;
				if (format[1] == '.')
				{
					for (size_t i = 2; format[i] >= '0' && format[i] <= '9'; ++i)
					{
						width *= 10;
						width += format[i] - '0';
					}
				}
				size_t i = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				//	size_t len
				if ((int)i < 0) {
					AuPutS("-");
					i = ((int)i * -1);
					sztoa(i, buffer, 10);
				}
				else {
					sztoa(i, buffer, 10);
					size_t len = strlen(buffer);
				}
				/*	while (len++ < width)
				puts("0");*/
				AuPutS(buffer);
			}
			else if (*format == 'c')
			{
				char c = va_arg(args, char);
				//char buffer[sizeof(size_t) * 8 + 1];
				//sztoa(c, buffer, 10);
				//puts(buffer);
				AuPutC(c);
			}
			else if (*format == 'x')
			{
				size_t x = va_arg(args, size_t);
				char buffer[sizeof(size_t) * 8 + 1];
				sztoa(x, buffer, 16);
				//puts("0x");
				AuPutS(buffer);
			}
			else if (*format == 's')
			{
				char* x = va_arg(args, char*);
				AuPutS(x);
			}
			else if (*format == 'f')
			{
				double x = va_arg(args, double);
				AuPutS(ftoa(x, 2));
			}
			else if (*format == '%')
			{
				AuPutS(".");
			}
			else
			{
				char buf[3];
				buf[0] = '%'; buf[1] = *format; buf[2] = '\0';
				AuPutS(buf);
			}
		}
		else
		{
			char buf[2];
			buf[0] = *format; buf[1] = '\0';
			AuPutS(buf);
		}
		++format;
	}
	va_end(args);

}

/*
 * AuConsoleEarlyEnable -- enables or disable early
 * mode text output
 * @param value -- boolean value
 */
void AuConsoleEarlyEnable(bool value) {
	aucon->early_mode = value;
}
