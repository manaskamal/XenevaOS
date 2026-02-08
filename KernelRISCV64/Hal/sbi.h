/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __RISCV64_SBI_H__
#define __RISCV64_SBI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SBI Return Structure */
typedef struct {
    long error;
    long value;
} sbiret_t;

/* Legacy SBI Extension IDs */
#define SBI_EXT_SET_TIMER           0x0
#define SBI_EXT_CONSOLE_PUTCHAR     0x1
#define SBI_EXT_CONSOLE_GETCHAR     0x2
#define SBI_EXT_CLEAR_IPI           0x3
#define SBI_EXT_SEND_IPI            0x4
#define SBI_EXT_REMOTE_FENCE_I      0x5
#define SBI_EXT_REMOTE_SFENCE_VMA   0x6
#define SBI_EXT_REMOTE_SFENCE_VMA_ASID 0x7
#define SBI_EXT_SHUTDOWN            0x8

/* SBI Call wrapper */
sbiret_t sbi_call(long ext, long fid, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5);

/* Wrapper functions */
void sbi_set_timer(uint64_t stime_value);
void sbi_console_putchar(int ch);
int sbi_console_getchar(void);
void sbi_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
