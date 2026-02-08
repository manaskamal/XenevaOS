.section .text
.global _start
.option norvc
.balign 4
_start:
    /* Call C++ Entry Point (a0=Handle, a1=SystemTable passed through) */
    .extern EfiMain
    call EfiMain
    ret

.section .data
.global _data_start
_data_start:
    /* Dummy data to ensure .data section has content */
    .quad 0xDEADBEEF
    
    .global _reloc_ptr
_reloc_ptr:
    .quad 0x80002000      /* A pointer to _data_start (VirtualAddress) */
    
    .skip 4080            /* Fill rest of 4KB page (16 + 4080 = 4096) */
