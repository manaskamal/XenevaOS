/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2026, Aryan Dadwal
 * All rights reserved.
 *
 * crt0_riscv64.s -- C runtime startup for RISC-V user space (XenevaOS)
 *
 * The kernel AuProcessEntUser calls our entrypoint with:
 *   a0 = argc
 *   a1 = argv pointer
 * and the stack pointer already set to the user stack.
 **/OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

ImageBase = 0x10000000;

SECTIONS
{
    /* The PE Header starts exactly at the ImageBase */
    . = ImageBase;
    
    .peheader : {
        *(.peheader)
    }

    /* .text section begins at ImageBase + 0x1000 (4KB) */
    . = ImageBase + 0x1000;
    _text = .;
    .text : {
        *(.text)
        *(.text.*)
    }
    . = ALIGN(4096);
    _etext = .;

    /* .data section begins at ImageBase + 0x20000 */
    . = ImageBase + 0x20000;
    _data = .;
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    .data : {
        *(.data)
        *(.data.*)
        *(.sdata)
        *(.sdata.*)
    }
    .bss : {
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
    }
    . = ALIGN(4096);
    _edata = .;

    /* .reloc section normally at ImageBase + 0x30000 if needed */
    . = ImageBase + 0x30000;
    _reloc = .;
    _ereloc = .;

    _end = .;
}
