.section .reloc, "a"
.global _reloc_start
_reloc_start:
    /* Base Relocation Block for .data (RVA 0x2000) */
    .long 0x2000          /* VirtualAddress (Page RVA) */
    .long 12              /* SizeOfBlock (Header 8 + Entry 2 + Pad 2 = 12) */
    
    /* Entry 1: Relocate _reloc_ptr at Offset 0x008 */
    /* Type 10 (IMAGE_REL_BASED_DIR64) | Offset 0x008 */
    .short 0xA008         /* 0xA = Type 10, 0x008 = Offset */
    
    /* Entry 2: Padding (Type 0) to align to 4 bytes */
    .short 0x0000
    
    /* Pad to fill .reloc section to 0x1000 bytes */
    /* Used 12 bytes. Need 4096 - 12 = 4084 bytes */
    .skip 4084
