/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#ifndef __AU_CONSOLE_H__
#define __AU_CONSOLE_H__

#include <aurora.h>

#define SCREEN_SETMODE    200
#define SCREEN_GETWIDTH   201
#define SCREEN_GETHEIGHT  202
#define SCREEN_GETBPP     203
#define SCREEN_SETBPP     204
#define SCREEN_UPDATE     205
#define SCREEN_GET_SCANLINE  206
#define SCREEN_GET_PITCH     207
#define SCREEN_REG_MNGR      208
#define SCREEN_GET_FB     209

typedef struct _aucon_ {
	uint32_t width;
	uint32_t height;
	uint32_t *buffer;
	uint32_t bpp;
	uint16_t scanline;
	uint32_t size;
	uint32_t pitch;
	bool early_mode;
}AuConsole;

/*
* AuConsoleInitialize -- initialize kernel direct screen
* console
* @param info -- Pointer to kernel boot info structure
*/
extern void AuConsoleInitialize(PKERNEL_BOOT_INFO info, bool early);

/*
* AuConsolePostInitialise -- initialise the post console process
* @param info -- pointer to kernel boot info structure
*/
extern void AuConsolePostInitialise(PKERNEL_BOOT_INFO info);

/*
* AuTextOut -- standard text printing function
* for entire kernel
* @param text -- text to output
*/
AU_EXTERN AU_EXPORT void AuTextOut(const char* text, ...);

/*
* AuConsoleEarlyEnable -- enables or disable early
* mode text output
* @param value -- boolean value
*/
void AuConsoleEarlyEnable(bool value);

#endif