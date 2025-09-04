.extern main
.extern _KeProcessExit

.global _xemain
_xemain:
     ldp x0,x1, [sp], #16
	 sub sp, sp,32
	 bl main
	 add sp, sp,32
	 bl _KeProcessExit