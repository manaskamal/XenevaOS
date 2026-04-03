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
ENTRY(_AuMain)

SECTIONS
{
  ImageBase = 0xFFFFFFFF80000000;
  
  /* Set VMA to ImageBase so objcopy outputs correctly */
  . = ImageBase;
  
  /* Force .peheader at start (file offset 0) */
  .peheader : AT(ImageBase) {
    KEEP(*(.peheader))
  }

  . = ImageBase + 0x1000;

  .text : AT(ImageBase + 0x1000) {
    _text = .;
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t.*)
    *(.rodata*)
    *(.srodata*)
    *(.srodata.*)
    . = ALIGN(4096);
    _etext = .;
  }
  
  . = ImageBase + 0x20000;

  .data : AT(ImageBase + 0x20000) {
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
    . = ALIGN(4096);
    _edata = .;
  }
  
  . = ImageBase + 0x30000;

  .reloc : AT(ImageBase + 0x30000) {
    _reloc = .;
    *(.reloc)
    . = ALIGN(4096);
    _ereloc = .;
  }

  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .rela.dyn : { *(.rela.dyn) }

  . = ALIGN(4096);
  _end = .;
  
  /DISCARD/ : { *(.hash) *(.gnu.hash) *(.note.gnu.build-id) *(.eh_frame) *(.eh_frame_hdr) }
}
