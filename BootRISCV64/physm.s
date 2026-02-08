	.attribute	4, 16
	.attribute	5, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zmmul1p0_zaamo1p0_zalrsc1p0_zca1p0_zcd1p0"
	.file	"physm.cpp"
	.text
	.globl	"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z" # -- Begin function ?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z
	.p2align	1
	.type	"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z",@function
"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z": # @"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z"
# %bb.0:
.Lpcrel_hi0:
	auipc	a3, %pcrel_hi(.L_MergedGlobals)
	srli	t1, a2, 1
	li	a2, 1
	addi	t4, a3, %pcrel_lo(.Lpcrel_hi0)
	add	a4, t1, a1
	sw	a2, 0(t4)
	sd	a1, 8(t4)
	sd	a1, 16(t4)
	sd	a4, 24(t4)
	sd	a4, 32(t4)
	ld	a2, 0(a0)
	beqz	a2, .LBB0_10
# %bb.1:
	ld	t0, 8(a0)
	li	a6, 7
	lui	a7, 1
	mv	a5, a1
	mv	t2, t0
	j	.LBB0_6
.LBB0_2:                                #   in Loop: Header=BB0_6 Depth=1
	ld	a5, 8(t4)
	ld	a1, 16(t4)
.LBB0_3:                                #   in Loop: Header=BB0_6 Depth=1
	sub	a2, a1, a5
	bgeu	a2, t1, .LBB0_10
# %bb.4:                                #   in Loop: Header=BB0_6 Depth=1
	ld	t0, 8(a0)
.LBB0_5:                                #   in Loop: Header=BB0_6 Depth=1
	ld	a2, 24(a0)
	ld	a3, 0(a0)
	add	t2, t2, a2
	sub	a2, t2, t0
	bgeu	a2, a3, .LBB0_10
.LBB0_6:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_8 Depth 2
	ld	a2, 24(t2)
	ld	a4, 56(t4)
	slli	t3, a2, 12
	add	a4, a4, t3
	sd	a4, 56(t4)
	lw	a4, 0(t2)
	bne	a4, a6, .LBB0_5
# %bb.7:                                #   in Loop: Header=BB0_6 Depth=1
	ld	a4, 8(t2)
	sd	t3, 48(t4)
	sd	a4, 40(t4)
	beqz	a2, .LBB0_3
.LBB0_8:                                #   Parent Loop BB0_6 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	ld	a5, 8(t4)
	ld	a1, 16(t4)
	sub	a3, a1, a5
	bgeu	a3, t1, .LBB0_3
# %bb.9:                                #   in Loop: Header=BB0_8 Depth=2
	addi	a3, a1, 8
	addi	a2, a2, -1
	sd	a3, 16(t4)
	sd	a4, 0(a1)
	add	a4, a4, a7
	bnez	a2, .LBB0_8
	j	.LBB0_2
.LBB0_10:
	ret
.Lfunc_end0:
	.size	"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z", .Lfunc_end0-"?XEInitialisePmmngr@@YAXU_efi_memory_map_@@PEAXK@Z"
                                        # -- End function
	.globl	"?XEPmmngrAllocate@@YA_KXZ"     # -- Begin function ?XEPmmngrAllocate@@YA_KXZ
	.p2align	1
	.type	"?XEPmmngrAllocate@@YA_KXZ",@function
"?XEPmmngrAllocate@@YA_KXZ":            # @"?XEPmmngrAllocate@@YA_KXZ"
# %bb.0:
.Lpcrel_hi1:
	auipc	a0, %pcrel_hi(.L_MergedGlobals)
	addi	a1, a0, %pcrel_lo(.Lpcrel_hi1)
	ld	a2, 8(a1)
	ld	a0, 16(a1)
	beq	a0, a2, .LBB1_2
# %bb.1:
	addi	a2, a0, -8
	ld	a3, 32(a1)
	sd	a2, 16(a1)
	ld	a0, -8(a0)
	addi	a2, a3, 8
	sd	a2, 32(a1)
	sd	a0, 0(a3)
	lw	a2, 0(a1)
	addi	a2, a2, 1
	sw	a2, 0(a1)
	ret
.LBB1_2:
	li	a0, 0
	ret
.Lfunc_end1:
	.size	"?XEPmmngrAllocate@@YA_KXZ", .Lfunc_end1-"?XEPmmngrAllocate@@YA_KXZ"
                                        # -- End function
	.globl	"?XEPmmngrFree@@YAX_K@Z"        # -- Begin function ?XEPmmngrFree@@YAX_K@Z
	.p2align	1
	.type	"?XEPmmngrFree@@YAX_K@Z",@function
"?XEPmmngrFree@@YAX_K@Z":               # @"?XEPmmngrFree@@YAX_K@Z"
# %bb.0:
.Lpcrel_hi2:
	auipc	a1, %pcrel_hi(.L_MergedGlobals+16)
	ld	a2, %pcrel_lo(.Lpcrel_hi2)(a1)
	addi	a3, a2, 8
	sd	a3, %pcrel_lo(.Lpcrel_hi2)(a1)
	sd	a0, 0(a2)
	ret
.Lfunc_end2:
	.size	"?XEPmmngrFree@@YAX_K@Z", .Lfunc_end2-"?XEPmmngrFree@@YAX_K@Z"
                                        # -- End function
	.globl	"?XEPmmngrList@@YAXXZ"          # -- Begin function ?XEPmmngrList@@YAXXZ
	.p2align	1
	.type	"?XEPmmngrList@@YAXXZ",@function
"?XEPmmngrList@@YAXXZ":                 # @"?XEPmmngrList@@YAXXZ"
	.cfi_startproc
# %bb.0:
	addi	sp, sp, -32
	.cfi_def_cfa_offset 32
	sd	ra, 24(sp)                      # 8-byte Folded Spill
	sd	s0, 16(sp)                      # 8-byte Folded Spill
	sd	s1, 8(sp)                       # 8-byte Folded Spill
	.cfi_offset ra, -8
	.cfi_offset s0, -16
	.cfi_offset s1, -24
.Lpcrel_hi3:
	auipc	a0, %pcrel_hi(.L_MergedGlobals)
	addi	s1, a0, %pcrel_lo(.Lpcrel_hi3)
	lw	a0, 0(s1)
	beqz	a0, .LBB3_3
# %bb.1:
.Lpcrel_hi4:
	auipc	a0, %pcrel_hi("??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@")
	addi	s0, a0, %pcrel_lo(.Lpcrel_hi4)
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
	ld	a0, 32(s1)
	addi	a1, a0, -8
	sd	a1, 32(s1)
	ld	a1, 0(a0)
	mv	a0, s0
	call	XEGuiPrint
	lw	a0, 0(s1)
	addiw	a0, a0, -1
	sw	a0, 0(s1)
	bnez	a0, .LBB3_2
.LBB3_3:
	ld	ra, 24(sp)                      # 8-byte Folded Reload
	ld	s0, 16(sp)                      # 8-byte Folded Reload
	ld	s1, 8(sp)                       # 8-byte Folded Reload
	.cfi_restore ra
	.cfi_restore s0
	.cfi_restore s1
	addi	sp, sp, 32
	.cfi_def_cfa_offset 0
	ret
.Lfunc_end3:
	.size	"?XEPmmngrList@@YAXXZ", .Lfunc_end3-"?XEPmmngrList@@YAXXZ"
	.cfi_endproc
                                        # -- End function
	.globl	"?XEGetAlstack@@YAPEA_KXZ"      # -- Begin function ?XEGetAlstack@@YAPEA_KXZ
	.p2align	1
	.type	"?XEGetAlstack@@YAPEA_KXZ",@function
"?XEGetAlstack@@YAPEA_KXZ":             # @"?XEGetAlstack@@YAPEA_KXZ"
# %bb.0:
.Lpcrel_hi5:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+24)
	ld	a0, %pcrel_lo(.Lpcrel_hi5)(a0)
	ret
.Lfunc_end4:
	.size	"?XEGetAlstack@@YAPEA_KXZ", .Lfunc_end4-"?XEGetAlstack@@YAPEA_KXZ"
                                        # -- End function
	.globl	"?XEGetAlstackptr@@YAPEA_KXZ"   # -- Begin function ?XEGetAlstackptr@@YAPEA_KXZ
	.p2align	1
	.type	"?XEGetAlstackptr@@YAPEA_KXZ",@function
"?XEGetAlstackptr@@YAPEA_KXZ":          # @"?XEGetAlstackptr@@YAPEA_KXZ"
# %bb.0:
.Lpcrel_hi6:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+32)
	ld	a0, %pcrel_lo(.Lpcrel_hi6)(a0)
	ret
.Lfunc_end5:
	.size	"?XEGetAlstackptr@@YAPEA_KXZ", .Lfunc_end5-"?XEGetAlstackptr@@YAPEA_KXZ"
                                        # -- End function
	.globl	"?XEReserveMemCount@@YA_KXZ"    # -- Begin function ?XEReserveMemCount@@YA_KXZ
	.p2align	1
	.type	"?XEReserveMemCount@@YA_KXZ",@function
"?XEReserveMemCount@@YA_KXZ":           # @"?XEReserveMemCount@@YA_KXZ"
# %bb.0:
.Lpcrel_hi7:
	auipc	a0, %pcrel_hi(.L_MergedGlobals)
	lwu	a0, %pcrel_lo(.Lpcrel_hi7)(a0)
	ret
.Lfunc_end6:
	.size	"?XEReserveMemCount@@YA_KXZ", .Lfunc_end6-"?XEReserveMemCount@@YA_KXZ"
                                        # -- End function
	.globl	"?XEGetPgStack@@YAPEA_KXZ"      # -- Begin function ?XEGetPgStack@@YAPEA_KXZ
	.p2align	1
	.type	"?XEGetPgStack@@YAPEA_KXZ",@function
"?XEGetPgStack@@YAPEA_KXZ":             # @"?XEGetPgStack@@YAPEA_KXZ"
# %bb.0:
.Lpcrel_hi8:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+8)
	ld	a0, %pcrel_lo(.Lpcrel_hi8)(a0)
	ret
.Lfunc_end7:
	.size	"?XEGetPgStack@@YAPEA_KXZ", .Lfunc_end7-"?XEGetPgStack@@YAPEA_KXZ"
                                        # -- End function
	.globl	"?XEGetStackPtr@@YAPEA_KXZ"     # -- Begin function ?XEGetStackPtr@@YAPEA_KXZ
	.p2align	1
	.type	"?XEGetStackPtr@@YAPEA_KXZ",@function
"?XEGetStackPtr@@YAPEA_KXZ":            # @"?XEGetStackPtr@@YAPEA_KXZ"
# %bb.0:
.Lpcrel_hi9:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+16)
	ld	a0, %pcrel_lo(.Lpcrel_hi9)(a0)
	ret
.Lfunc_end8:
	.size	"?XEGetStackPtr@@YAPEA_KXZ", .Lfunc_end8-"?XEGetStackPtr@@YAPEA_KXZ"
                                        # -- End function
	.globl	"?XEGetPhysicalStart@@YA_KXZ"   # -- Begin function ?XEGetPhysicalStart@@YA_KXZ
	.p2align	1
	.type	"?XEGetPhysicalStart@@YA_KXZ",@function
"?XEGetPhysicalStart@@YA_KXZ":          # @"?XEGetPhysicalStart@@YA_KXZ"
# %bb.0:
.Lpcrel_hi10:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+40)
	ld	a0, %pcrel_lo(.Lpcrel_hi10)(a0)
	ret
.Lfunc_end9:
	.size	"?XEGetPhysicalStart@@YA_KXZ", .Lfunc_end9-"?XEGetPhysicalStart@@YA_KXZ"
                                        # -- End function
	.globl	"?XEGetPhysicalSize@@YA_KXZ"    # -- Begin function ?XEGetPhysicalSize@@YA_KXZ
	.p2align	1
	.type	"?XEGetPhysicalSize@@YA_KXZ",@function
"?XEGetPhysicalSize@@YA_KXZ":           # @"?XEGetPhysicalSize@@YA_KXZ"
# %bb.0:
.Lpcrel_hi11:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+48)
	ld	a0, %pcrel_lo(.Lpcrel_hi11)(a0)
	ret
.Lfunc_end10:
	.size	"?XEGetPhysicalSize@@YA_KXZ", .Lfunc_end10-"?XEGetPhysicalSize@@YA_KXZ"
                                        # -- End function
	.globl	"?XEGetRamSize@@YA_KXZ"         # -- Begin function ?XEGetRamSize@@YA_KXZ
	.p2align	1
	.type	"?XEGetRamSize@@YA_KXZ",@function
"?XEGetRamSize@@YA_KXZ":                # @"?XEGetRamSize@@YA_KXZ"
# %bb.0:
.Lpcrel_hi12:
	auipc	a0, %pcrel_hi(.L_MergedGlobals+56)
	ld	a0, %pcrel_lo(.Lpcrel_hi12)(a0)
	ret
.Lfunc_end11:
	.size	"?XEGetRamSize@@YA_KXZ", .Lfunc_end11-"?XEGetRamSize@@YA_KXZ"
                                        # -- End function
	.type	"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@",@object # @"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@"
	.section	".rodata.str1.1.??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@","aMSG",@progbits,1,"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@",comdat
	.weak	"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@"
"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@":
	.asciz	"Address -> %x \n"
	.size	"??_C@_0BA@KEPMIDLN@Address?5?9?$DO?5?$CFx?5?6?$AA@", 16

	.type	.L_MergedGlobals,@object        # @_MergedGlobals
	.local	.L_MergedGlobals
	.comm	.L_MergedGlobals,64,8
	.globl	"?allocatedCount@@3IA"
"?allocatedCount@@3IA" = .L_MergedGlobals
	.size	"?allocatedCount@@3IA", 4
	.globl	"?pagestack@@3PEA_KEA"
"?pagestack@@3PEA_KEA" = .L_MergedGlobals+8
	.size	"?pagestack@@3PEA_KEA", 8
	.globl	"?stackptr@@3PEA_KEA"
"?stackptr@@3PEA_KEA" = .L_MergedGlobals+16
	.size	"?stackptr@@3PEA_KEA", 8
	.globl	"?allocatedStack@@3PEA_KEA"
"?allocatedStack@@3PEA_KEA" = .L_MergedGlobals+24
	.size	"?allocatedStack@@3PEA_KEA", 8
	.globl	"?allocatedPtr@@3PEA_KEA"
"?allocatedPtr@@3PEA_KEA" = .L_MergedGlobals+32
	.size	"?allocatedPtr@@3PEA_KEA", 8
	.globl	"?usableRam@@3_KA"
"?usableRam@@3_KA" = .L_MergedGlobals+40
	.size	"?usableRam@@3_KA", 8
	.globl	"?usableSize@@3_KA"
"?usableSize@@3_KA" = .L_MergedGlobals+48
	.size	"?usableSize@@3_KA", 8
	.globl	"?ramSize@@3_KA"
"?ramSize@@3_KA" = .L_MergedGlobals+56
	.size	"?ramSize@@3_KA", 8
	.ident	"clang version 21.1.8"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym .L_MergedGlobals
