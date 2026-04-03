/**
* BSD 2-Clause License
*
* Copyright (c) 2026, Aryan Dadwal
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
.section .peheader, "a"
.global _start_header
_start_header:

/* DOS Header */
.ascii "MZ"             /* e_magic */
.skip 58                /* ... */
.long 128               /* e_lfanew (Offset to PE Header) */

/* DOS Stub */
.skip 64                /* Stub code / padding */

/* PE Header */
.ascii "PE\0\0"         /* Signature */

/* COFF Header */
.short 0x5064           /* Machine (RISCV64) */
.short 3                /* NumberOfSections */
.long 0                 /* TimeDateStamp */
.long 0                 /* PointerToSymbolTable */
.long 0                 /* NumberOfSymbols */
.short 240              /* SizeOfOptionalHeader (PE32+) */
.short 0x2022           /* Characteristics */

/* Optional Header (PE32+) */
.short 0x20B            /* Magic */
.byte 1                 /* MajorLinkerVersion */
.byte 0                 /* MinorLinkerVersion */
.long _etext - _text    /* SizeOfCode */
.long _edata - _data    /* SizeOfInitializedData */
.long 0                 /* SizeOfUninitializedData */
.long _kernel_entry - ImageBase /* AddressOfEntryPoint */
.long _text - ImageBase /* BaseOfCode */
.quad 0xFFFFFFFF80000000        /* ImageBase */
.long 4096              /* SectionAlignment */
.long 512               /* FileAlignment */
.short 1, 0             /* Major/Minor OS Version */
.short 0, 0             /* Major/Minor Image Version */
.short 1, 0             /* Major/Minor Subsystem Version */
.long 0                 /* Win32VersionValue */
.long _end - ImageBase   /* SizeOfImage */
.long _text - ImageBase /* SizeOfHeaders */
.long 0                 /* CheckSum */
.short 1                /* Subsystem (Native) */
.short 0                /* DllCharacteristics */
.quad 0x100000          /* SizeOfStackReserve */
.quad 0x10000           /* SizeOfStackCommit */
.quad 0x100000          /* SizeOfHeapReserve */
.quad 0x10000           /* SizeOfHeapCommit */
.long 0                 /* LoaderFlags */
.long 16                /* NumberOfRvaAndSizes */

/* Data Directories */
.long 0, 0 /* Export */
.long 0, 0 /* Import */
.long 0, 0 /* Resource */
.long 0, 0 /* Exception */ 
.long 0, 0 /* Certificate */
.long 0, 0 /* BaseReloc (Disabled) */
.skip 10*8  /* Remaining (10 entries * 8 bytes) */

/* Section Table (3 sections) */
/* .text */
.ascii ".text\0\0\0"
.long _etext - _text      /* VirtualSize */
.long 0x1000              /* VirtualAddress (Hardcoded) */
.long _etext - _text      /* SizeOfRawData */
.long 0x1000              /* PointerToRawData */
.long 0, 0, 0             /* PtrReloc, PtrLine, CoReloc/Line (12 bytes) */
.long 0x60000020          /* Characteristics (CNT_CODE | MEM_EXECUTE | MEM_READ) */

/* .data */
.ascii ".data\0\0\0"
.long _edata - _data      /* VirtualSize */
.long 0x20000             /* VirtualAddress (Hardcoded) */
.long _edata - _data      /* SizeOfRawData */
.long 0x20000             /* PointerToRawData */
.long 0, 0, 0             /* PtrReloc, PtrLine, CoReloc/Line (12 bytes) */
.long 0xC0000040          /* Characteristics (CNT_INIT_DATA | MEM_READ | MEM_WRITE) */

/* .reloc */
.ascii ".reloc\0\0"
.long _ereloc - _reloc    /* VirtualSize */
.long 0x30000             /* VirtualAddress (Hardcoded) */
.long _ereloc - _reloc    /* SizeOfRawData */
.long 0x30000             /* PointerToRawData */
.long 0, 0, 0             /* PtrReloc, PtrLine, CoReloc/Line (12 bytes) */
.long 0x42000040          /* Characteristics (CNT_INIT_DATA | DISCARDABLE | READ) */
