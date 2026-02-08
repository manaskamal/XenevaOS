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

