/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __KERNEL_RISCV64_H__
#define __KERNEL_RISCV64_H__

#include <aurora.h>

/*
 * AuLittleBootUsed -- check if little boot protocol
 * is used
 */
AU_EXTERN AU_EXPORT bool AuLittleBootUsed();

/*
 * RISCV64PCIeInitialize -- intialize pcie controller
 */
extern void RISCV64PCIeInitialize();
#endif
