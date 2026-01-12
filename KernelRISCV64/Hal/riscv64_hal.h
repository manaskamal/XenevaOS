/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __RISCV64_HAL_H__
#define __RISCV64_HAL_H__

#include <aurora.h>

#ifdef __cplusplus
extern "C" {
#endif

void RISCV64CpuInitialize();
void RISCV64CPUPostInitialize(KERNEL_BOOT_INFO* info);
void AuRISCV64BoardInitialize();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
    void AuRISCV64TrapHandler(void* stack_frame);
#ifdef __cplusplus
}
#endif

#endif
