.extern main
.extern _KeProcessExit

.global _aumain
_aumain:
     ldp x0,x1, [sp]
	// sub sp, sp,#32
	 bl main
	// add sp, sp,32
	 bl _KeProcessExit


.global _printstack
_printstack:
     mov x0, sp
     ret
