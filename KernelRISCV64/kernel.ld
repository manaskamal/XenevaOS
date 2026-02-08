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
