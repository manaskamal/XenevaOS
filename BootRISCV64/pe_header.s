
.section .peheader, "a"
.global _start_header
_start_header:


/* DOS Header */
.ascii "MZ"             /* e_magic */
.skip 58                /* ... */
.long 128               /* e_lfanew (Offset to PE Header) */

/* DOS Stub (64 bytes of padding to reach offset 128) */
.skip 64

/* PE Header at offset 0x80 (128) */
.ascii "PE\0\0"         /* Signature */

/* COFF Header (20 bytes) */
.short 0x5064           /* Machine (RISCV64) */
.short 3                /* NumberOfSections */
.long 0x65AE1234         /* TimeDateStamp (arbitrary) */
.long 0                 /* PointerToSymbolTable */
.long 0                 /* NumberOfSymbols */
.short 240              /* SizeOfOptionalHeader (PE32+) */
.short 0x2022           /* Characteristics (Exec | LargeAware | DLL) */

/* Optional Header (PE32+) - 240 bytes */
.short 0x20B            /* Magic (PE32+) */
.byte 1                 /* MajorLinkerVersion */
.byte 0                 /* MinorLinkerVersion */
.long 0x8000            /* SizeOfCode (.text size) */
.long 0x5000            /* SizeOfInitializedData (.data + .reloc size) */
.long 0                 /* SizeOfUninitializedData */
.long 0x1000            /* AddressOfEntryPoint (_start is at 0x1000) */
.long 0x1000            /* BaseOfCode */
.quad 0x80200000        /* ImageBase (2GB + 2MB Standard) */
.long 0x1000            /* SectionAlignment */
.long 0x1000            /* FileAlignment (4KB conservative) */
.short 1, 0             /* Major/Minor OS Version */
.short 0, 0             /* Major/Minor Image Version */
.short 1, 0             /* Major/Minor Subsystem Version */
.long 0                 /* Win32VersionValue */
.long 0xE000            /* SizeOfImage (Hdr+Text+Data+Reloc = 0xE000) */
.long 0x1000            /* SizeOfHeaders (first section starts at 0x1000) */
.long 0                 /* CheckSum */
.short 10               /* Subsystem (EFI Application) */
.short 0                /* DllCharacteristics (None) */
.quad 0x100000          /* SizeOfStackReserve */
.quad 0x10000           /* SizeOfStackCommit */
.quad 0x100000          /* SizeOfHeapReserve */
.quad 0x10000           /* SizeOfHeapCommit */
.long 0                 /* LoaderFlags */
.long 16                /* NumberOfRvaAndSizes */

/* Data Directories (16 entries x 8 bytes = 128 bytes) */
.long 0, 0              /* 0: Export */
.long 0, 0              /* 1: Import */
.long 0, 0              /* 2: Resource */
.long 0, 0              /* 3: Exception */
.long 0, 0              /* 4: Certificate */
.long 0, 0              /* 5: BaseReloc (Disabled - Match Kernel) */
.long 0, 0              /* 6: Debug */
.long 0, 0              /* 7: Architecture */
.long 0, 0              /* 8: Global Ptr */
.long 0, 0              /* 9: TLS */
.long 0, 0              /* 10: Load Config */
.long 0, 0              /* 11: Bound Import */
.long 0, 0              /* 12: IAT */
.long 0, 0              /* 13: Delay Import */
.long 0, 0              /* 14: CLR Runtime */
.long 0, 0              /* 15: Reserved */

/* Section Table (3 sections x 40 bytes = 120 bytes) */

/* .text section */
.ascii ".text\0\0\0"    /* Name (8 bytes) */
.long 0x8000            /* VirtualSize */
.long 0x1000            /* VirtualAddress (RVA) */
.long 0x8000            /* SizeOfRawData */
.long 0x1000            /* PointerToRawData (file offset: 0x1000) */
.long 0                 /* PointerToRelocations */
.long 0                 /* PointerToLinenumbers */
.short 0                /* NumberOfRelocations */
.short 0                /* NumberOfLinenumbers */
.long 0x60000020        /* Characteristics (CODE|EXECUTE|READ) */

/* .data section */
.ascii ".data\0\0\0"    /* Name (8 bytes) */
.long 0x4000            /* VirtualSize */
.long 0x9000            /* VirtualAddress (RVA) */
.long 0x4000            /* SizeOfRawData */
.long 0x9000            /* PointerToRawData (file offset: 0x9000) */
.long 0                 /* PointerToRelocations */
.long 0                 /* PointerToLinenumbers */
.short 0                /* NumberOfRelocations */
.short 0                /* NumberOfLinenumbers */
.long 0xC0000040        /* Characteristics (INITIALIZED_DATA|READ|WRITE) */

/* .reloc section */
.ascii ".reloc\0\0"     /* Name (8 bytes) */
.long 0x1000            /* VirtualSize */
.long 0xD000            /* VirtualAddress (RVA) */
.long 0x1000            /* SizeOfRawData */
.long 0xD000            /* PointerToRawData (file offset: 0xD000) */
.long 0                 /* PointerToRelocations */
.long 0                 /* PointerToLinenumbers */
.short 0                /* NumberOfRelocations */
.short 0                /* NumberOfLinenumbers */
.long 0x40000040        /* Characteristics (INITIALIZED_DATA|READ) - No DISCARDABLE */

/* Padding to fill header to 0x1000 bytes */
/* DOS Header (64) + DOS Stub (64) + PE Sig (4) + COFF (20) + Optional (240) + Sections (120) = 512 bytes */
/* Need 0x1000 - 512 = 3584 bytes of padding */
.skip 3584

