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
OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
  ImageBase = 0x80200000;
  . = ImageBase;

  /* PE Header at offset 0x0000 */
  .peheader : AT(0) {
    KEEP(*(.peheader))
  }

  /* Force .text to start at VMA ImageBase + 0x1000 */
  . = ImageBase + 0x1000;
  
  /* .text starts at 0x1000 and is now larger (~0x6000 bytes) */
  .text : AT(0x1000) {
    _text = .;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    *(.rodata*)
    *(.srodata*)
  }
  
  /* Pad .text to align to 0x9000 (32KB for code) */
  . = ImageBase + 0x9000;
  _etext = .;

  .data : AT(0x9000) {
    _data = .;
    *(.got.plt)
    *(.got)
    *(.data*)
    *(.sdata)
    *(.sbss)
    *(.scommon)
    *(.dynbss)
    *(.bss)
    *(COMMON)
    *(.rel.local)
  }
  
  /* Give .data 16KB space */
  . = ImageBase + 0xD000;
  _edata = .;

  .reloc : AT(0xD000) {
    _reloc = .;
    *(.reloc)
    _ereloc = .;
  }
  
  . = ImageBase + 0xE000;
  _end = .;
  
  /* DISCARD all ELF-specific sections that would corrupt the PE layout */
  /DISCARD/ : { 
    *(.dynamic) 
    *(.rela.dyn) 
    *(.dynsym) 
    *(.dynstr) 
    *(.hash) 
    *(.gnu.hash) 
    *(.note.gnu.build-id) 
    *(.rela.*)
    *(.rel.*)
    *(.interp)
    *(.comment)
    *(.note.*)
  }
}

