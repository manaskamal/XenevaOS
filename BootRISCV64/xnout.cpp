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

#include "xnout.h"
#include "xnldr.h"
#include "clib.h"
#include "video.h"

EFI_SYSTEM_TABLE* gSystemTable __attribute__((section(".data")));



/*
 * XEClearScreen -- clears the screen
 */
void XEClearScreen(EFI_SYSTEM_TABLE* SystemTable) {
	SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

/*
 * XESetCursor -- Set the cursor on or off
 * @param on -- boolean value
 */
EFI_STATUS XESetCursor(const unsigned char on) {
	return gSystemTable->ConOut->EnableCursor(gSystemTable->ConOut, on);
}

/*
 * XEGetCurrentCursorPos -- returns the current
 * cursor pos to given variable
 * @param Col -- Pointer to where Column value should be stored
 * @param Row -- Pointer to where Row value should be stored
 */
void XEGetCurrentCursorPos(int* Col, int* Row) {
	*Col = gSystemTable->ConOut->Mode->CursorColumn;
	*Row = gSystemTable->ConOut->Mode->CursorRow;
}

/*
 * XESetCurrentCursorPos -- Set the current cursor pos
 * @param Col -- Column value
 * @param Row -- Row value
 */
EFI_STATUS XESetCurrentCursorPos(const int Col, const int Row) {
	return gSystemTable->ConOut->SetCursorPosition(gSystemTable->ConOut, Col, Row);
}

/*
 * XESetTextAttribute -- Set text and background color
 * @param Back -- Background color
 * @param Fore -- Foreground color
 */
EFI_STATUS XESetTextAttribute(const int Back, const int Fore) {
	return gSystemTable->ConOut->SetAttribute(gSystemTable->ConOut, Fore | Back);
}


void XEPutChar(EFI_SYSTEM_TABLE* SystemTable, const int ch) {
	unsigned short text[2];
	text[0] = (unsigned short)ch;
	text[1] = 0;
	SystemTable->ConOut->OutputString(SystemTable->ConOut, text);
}

// extern "C" void store_a0_a7(uint64_t * buffer); // Removed

/*
 * XEPrintf -- print function that will
 * show output to the screen
 * @param fmt -- format to print
 */
/*
 * XEPrintf -- print function that will
 * show output to the screen
 * @param fmt -- format to print
 */
int XEPrintf(EFI_SYSTEM_TABLE* SystemTable, wchar_t* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);

	unsigned short out[1024];
	int o = 0;
	int c, sign, width, precision, lmodifier;
	unsigned char ljust, alt, lzeroes;

	while ((c = *fmt++)) {
		if (c != '%' || *fmt == '%') {
			out[o++] = c;
			fmt += (c == '%');
			continue;
		}
		if ((c = (uint8_t)*fmt++) == '\0')
			return -1;
		ljust = alt = lzeroes = FALSE;
		sign = 0;
		for (;;) {
			if (c == '-') {
				ljust = TRUE;
				lzeroes = FALSE;
			}
			else if (c == '+')
				sign = '+';
			else if (c == ' ') {
				if (!sign)
					sign = ' ';
			}
			else if (c == '#')
				alt = TRUE;
			else if (c == '0') {
				if (!ljust)
					lzeroes = TRUE;
			}
			else
				break;

			if ((c = (uint8_t)*fmt++) == '\0')
				return -1;
		}

		width = -1;
		if (is_digit(c)) {
			width = 0;
			while (is_digit(c)) {
				width = width * 10 + (c - '0');
				if ((c = (uint8_t)*fmt++) == '\0')
					return -1;
			}
		}
		else if (c == '*') {
			width = va_arg(vl, int);
		}

		if (c == '[') {
			if (o > 0) {
				out[o] = 0;
				SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
				o = 0;
			}
			/**uint32 attr = *(unsigned long*)vl;
			vl += sizeof(unsigned long);**/
			// XESetTextAttribute(0x000000F0, 0x0000000F); //attr &
			continue;
		}
		//restore the default text attribute
		if (c == ']') {
			if (o > 0) {
				out[o] = 0;
				SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
				o = 0;
			}
			// XESetTextAttribute(EFI_BACKGROUND_BLACK, EFI_LIGHTGRAY);
			continue;
		}

		precision = -1;
		if (c == '.') {
			if ((c = (uint8_t)*fmt++) == '\0')
				return -1;
			precision = 0;
			lzeroes = FALSE;
			if (is_digit(c)) {
				while (is_digit(c)) {
					precision = precision * 10 + (c - '0');
					if ((c = (uint8_t)*fmt++) == '\0')
						return -1;
				}
			}
			else if (c == '*') {
				precision = va_arg(vl, int);
				if ((c = *fmt++) == '\0')
					return -1;
			}
		}

		lmodifier = 0;
		if (c == 'h') {
			if (*fmt == 'h') {
				fmt++;
				lmodifier = 'H';
			}
			else
				lmodifier = c;
		}
		else if (wstrchr((wchar_t*)L"jzt", c))
			lmodifier = c;
		if (lmodifier)
			if ((c = (uint8_t)*fmt++) == '\0')
				return -1;

		if (c == 'i')
			c = 'd';
		if (!wstrchr((wchar_t*)L"douxXcsp", c))
			return -1;

		if (c == 'c') {
			int ch = va_arg(vl, int);
			if (!ljust)
				while (width > 1) {
					out[o++] = ' ';
					width--;
				}
			out[o++] = ch;

			if (ljust)
				while (width > 1) {
					out[o++] = ' ';
					width--;
				}
			continue;
		}
		else if (c == 's') {
			int len, i;
			wchar_t* s = va_arg(vl, wchar_t*);

            if (!s) s = (wchar_t*)L"(null)"; 

			if (precision < 0)
				len = wstrlen(s);
			else {
				len = 0;
				while (len < precision)
					if (s[len]) len++;
					else break;
			}

			if (!ljust) {
				while (width > len) {
					out[o++] = ' ';
					width--;
				}
			}

			i = len;
			while (i--)
				out[o++] = *s++;

			if (ljust) {
				while (width > len) {
					out[o++] = ' ';
					width--;
				}
			}
			continue;
		}
		else {
            // Unsigned handling (covers d, o, u, x, X, p)
            // Note: 'd' logic handles signed separately below, but we read as unsigned/ulong first
			unsigned long long v; 
            if (c == 'p') {
                v = (unsigned long long)va_arg(vl, void*);
            } else {
                v = va_arg(vl, unsigned long long); // RISC-V 64, assume 64-bit args? 
                // Wait, va_arg(vl, unsigned) might fetch 32-bit.
                // In RISC-V 64, all args are extended to 64-bit in registers or stack.
                // It is safer to read as largest integer type if we aren't sure of modifier.
                // But the code uses 'lmodifier'.
            }

            // Correction: Read based on modifier?
            // The original code read `*(unsigned*)vl` (32-bit) then `vl+=8` (64-bit slot).
            // This implies the original expectation was 64-bit slot consumption.
            // On RISC-V 64, int is 32-bit but passed in 64-bit register.
            // va_arg(vl, int) works correctly.
            // va_arg(vl, long long) works correctly.
            
            // Let's re-read the original logic:
            // "unsigned v = *(unsigned*)vl; vl += 8;"
            // It reads 32-bit from the 64-bit slot.
            
            // Refined Logic using va_arg:
            if (lmodifier == 'l') { // %ld
                 v = va_arg(vl, unsigned long);
            } else if (lmodifier == 'z') {
                 v = va_arg(vl, size_t);
            } else {
                 if (c == 'p') {
                     v = (unsigned long long)va_arg(vl, void*);
                 } else {
                     v = va_arg(vl, unsigned int); // Default to int/uint
                 }
            }

			char s[22]; // Increased buffer size for 64-bit numbers
			char* p = s + sizeof(s);
			unsigned base = (c == 'p') ? 16 : 10;
			char* digits = (char*)"0123456789abcdef";
			char* hexpfx = (char*)NULL;
			int dcnt;
			int len;
    

			if (c == 'o')
				base = 8;
			else if (to_upper(c) == 'X') {
				base = 16;
				if (c == 'X')
					digits = (char*)"0123456789ABCDEF";
				if (alt && v)
					hexpfx = (char*)((c == 'X') ? "0X" : "0x");
			}

			if (c != 'd') {
                // Unsigned
                // Casts handled by va_arg read above?
                // Truncate if needed
				if (lmodifier == 'H')
					v = (uint8_t)v;
				else if (lmodifier == 'h')
					v = (unsigned short)v;
				sign = 0;
			}
			else {
                // Signed 'd'
                long long sv = (long long)v;
                // If it was read as unsigned int, sign extension might be missing if we simply cast?
                // Actually, if we passed a negative int, va_arg(vl, int) returns it as int.
                // We cast to long long sv.
                
                // Let's fix the read logic for 'd' specifically:
                 if (lmodifier == 0) {
                     int iv = (int)v; // Re-interpret lower bits?
                     // No, if I passed -1, v (unsigned int) has valid bits.
                     // (int)v is -1.
                     sv = (int)v;
                 }
                 
				if (lmodifier == 'H')
					sv = (signed char)sv;
				else if (lmodifier == 'h')
					sv = (short)sv;
                    
				if (sv < 0) {
					v = -sv;
					sign = '-';
				} else {
                    v = sv;
                }
			}


			unsigned long long tmp = v;
			do {
				*--p = digits[tmp % base];
				tmp /= base;
			} while (tmp);
			dcnt = s + sizeof(s) - p;

			if (precision < 0)
				precision = 1;
			else if ((v == 0) && (precision == 0))
				dcnt = 0;

			if (alt && (c == 'o'))
				if (((v == 0) && (precision == 0)) || (v && (precision <= dcnt)))
					precision = dcnt + 1;

			if (precision < dcnt)
				precision = dcnt;


			len = (sign != 0) + (hexpfx != NULL) * 2 + precision;

			if (!ljust && !lzeroes)
				while (width > len) {
					out[o++] = ' ';
					width--;
				}

			if (sign)
				out[o++] = sign;
			else if (hexpfx) {
				out[o++] = hexpfx[0];
				out[o++] = hexpfx[1];
			}

			if (!ljust && lzeroes)
				while (width > len) {
					out[o++] = '0';
					width--;
				}

			while (precision-- > dcnt)
				out[o++] = '0';

			while (dcnt--)
				out[o++] = *p++;

			if (ljust)
				while (width > len) {
					out[o++] = ' ';
					width--;
				}

			continue;
		}
	}
    
    va_end(vl);
	out[o++] = 0;
	SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
	return 0;
}


/*
 * XEGuiPrint -- print formated text using graphics
 * @param format -- formated string
 */
/*
 * XEGuiPrint -- print formated text using UART for early kernel debugging
 * @param format -- formated string
 */
extern "C" void XEGuiPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);

    volatile uint8_t* uart = (volatile uint8_t*)0x10000000;

	while (*format != '\0') {
		if (*format == '%') {
			++format;
			if (*format == 'd' || *format == 'x' || *format == 's') {
                if (*format == 's') {
                   char* s = va_arg(args, char*);
                   while(*s) {
                       while ((uart[5] & 0x20) == 0);
                       uart[0] = *s++;
                   }
                } else if (*format == 'd' || *format == 'x') {
                    size_t val = va_arg(args, size_t);
                    char buf[32];
                    sztoa(val, buf, (*format == 'x' ? 16 : 10));
                    char* p = buf;
                    while(*p) {
                       while ((uart[5] & 0x20) == 0);
                       uart[0] = *p++;
                    }
                }
			}
			else if (*format == 'c') {
				char c = va_arg(args, int);
                while ((uart[5] & 0x20) == 0);
                uart[0] = c;
			}
			else if (*format == '%') {
                while ((uart[5] & 0x20) == 0);
                uart[0] = '%';
			}
		}
		else
		{
            while ((uart[5] & 0x20) == 0);
            uart[0] = *format;
		}
		++format;
	}
    va_end(args);
}
