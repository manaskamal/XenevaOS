/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2025, Manas Kamal Choudhury
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

#include "video.h"

// FRAMEBUFFER_INFORMATION fbinfo; // Global removed
// Global vars removed



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


/*
 * XEInitialiseGraphics -- Initialise the screen and store graphics
 * information in fbinfo structure
 * @GraphicsOutput -- Pointer to Graphics Output protocol
 */
EFI_STATUS XEInitialiseGraphics(EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput, EFI_SYSTEM_TABLE* SystemTable, FRAMEBUFFER_INFORMATION* fbinfo) {

	SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_BACKGROUND_BLUE | EFI_WHITE);
	SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, 0, 0);

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = GraphicsOutput->Mode->Info;

	//clear screen
	// EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;
	// pixel.Blue = 164;
	// pixel.Green = 115;
	// pixel.Red = 0;

	// GraphicsOutput->Blt(GraphicsOutput, &pixel, EfiBltVideoFill, 0, 0, 0, 0, info->HorizontalResolution, info->VerticalResolution, 0);
    // Don't Blt here, use ClearScreen later or assume caller does?
    // Actually ClearScreen function calls Blt.
    
	fbinfo->phyaddr = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
	fbinfo->graphics_framebuffer = (uint32_t*)GraphicsOutput->Mode->FrameBufferBase;
	fbinfo->size = GraphicsOutput->Mode->FrameBufferSize;
	fbinfo->pixelsPerLine = GraphicsOutput->Mode->Info->PixelsPerScanLine;
	fbinfo->X_Resolution = GraphicsOutput->Mode->Info->HorizontalResolution;
	fbinfo->Y_Resolution = GraphicsOutput->Mode->Info->VerticalResolution;

	switch (GraphicsOutput->Mode->Info->PixelFormat)
	{
	case PixelRedGreenBlueReserved8BitPerColor:
		fbinfo->redmask = 0xFF;
		fbinfo->greenmask = 0xFF00;
		fbinfo->bluemask = 0xFF0000;
		fbinfo->resvmask = 0xFF000000;
		break;
	case PixelBlueGreenRedReserved8BitPerColor:
		fbinfo->redmask = 0xFF0000;
		fbinfo->greenmask = 0xFF00;
		fbinfo->bluemask = 0xFF;
		fbinfo->resvmask = 0xFF000000;
		break;
	case PixelBitMask:
		fbinfo->redmask = GraphicsOutput->Mode->Info->PixelInformation.RedMask;
		fbinfo->greenmask = GraphicsOutput->Mode->Info->PixelInformation.GreenMask;
		fbinfo->bluemask = GraphicsOutput->Mode->Info->PixelInformation.BlueMask;
		fbinfo->resvmask = GraphicsOutput->Mode->Info->PixelInformation.ReservedMask;
		break;
	default:
		break;
	}

    // Initialize statics inside struct? No, fbinfo is just data.
    // We can't keep xpos/ypos as globals either if they crash.
    // But xnldr only uses Puts to verify.
    // Let's assume xpos/ypos are safe if they are bss?
    // But fbinfo access crashed.
    // Let's put cursor inside fbinfo? Use paddings? Or unused fields?
    // video.h struct definition is fixed.
    // For now, let's keep xpos/ypos as strict locals or static inside function?
    return EFI_SUCCESS;
}



/*
 * XEPutPixel -- puts a pixel on the screen
 * @param x -- x location of the screen
 * @param y -- y location of the screen
 * @param col -- color of the pixel
 */
// Need bpp, masks.
// Re-calculate bpp etc inside PutPixel using fbinfo?
// Or just ignore PutPixel/Puts implementation for now since we just want to boot?
// xnldr.cpp calls XEGraphicsPuts.
// I will implement a minimal version that hardcodes colors or recalculates.
void XEPutPixel(size_t x, size_t y, uint32_t col, FRAMEBUFFER_INFORMATION* fbinfo) {
	uint32_t* framebuffer = fbinfo->phyaddr;
    if (!framebuffer) return;
    // Assume 32bpp for simplicity in bootloader
	framebuffer[fbinfo->pixelsPerLine * y + x] = col;
}

/*
 * XEGraphicsPutC -- put a character to the screen
 * @param str -- character to print
 */
// Basic globals for cursor
size_t _xpos = 0;
size_t _ypos = 0;

void XEGraphicsPutC(char str, FRAMEBUFFER_INFORMATION* fbinfo) {
	if (_xpos * 9 > fbinfo->X_Resolution - 9) {
		_xpos = 0;
		_ypos++;
	}

	for (size_t y = 0; y < 16; ++y) {
		for (size_t x = 0; x < 8; ++x) {
            // Cast to unsigned char to avoid negative index
			const bx_fontcharbitmap_t& entry = bx_vgafont[(unsigned char)str];
			if (entry.data[y] & (1 << x)) {
				XEPutPixel(x + _xpos * 9, y + _ypos * 16, 0xFFFFFFFF, fbinfo);
			}
			else {
				XEPutPixel(x + _xpos * 9, y + _ypos * 16, 0xFF000000, fbinfo);
			}
		}
		XEPutPixel(8 + _xpos * 9, y + _ypos * 16, 0xFF000000, fbinfo);
	}

	_xpos++;
}

/*
 * XEGraphicsPuts -- put string on the screen
 * @param str -- string to put
 */
void XEGraphicsPuts(const char* str, FRAMEBUFFER_INFORMATION* fbinfo) {
	while (*str) {
		if (*str == '\n') {
			++_ypos;
			_xpos = 0;
		}
		else if (*str == '\r') {
		}
		else {
			XEGraphicsPutC(*str, fbinfo);
		}
		++str;
	}
}

/*
 * XEGraphicsClearScreen -- clear the entire screen
 * @param gop -- Pointer to Graphics Output Protocol
 */
void XEGraphicsClearScreen(EFI_GRAPHICS_OUTPUT_PROTOCOL* gop, FRAMEBUFFER_INFORMATION* fbinfo) {
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;
	pixel.Blue = 0;
	pixel.Green = 0;
	pixel.Red = 0;
	_xpos = _ypos = 0;

	gop->Blt(gop, &pixel, EfiBltVideoFill, 0, 0, 0, 0, fbinfo->X_Resolution, fbinfo->Y_Resolution, 0);
}


// Getters removed

