/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "sbi.h"

sbiret_t sbi_call(long ext, long fid, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) {
    sbiret_t ret;
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    register long a3 asm("a3") = arg3;
    register long a4 asm("a4") = arg4;
    register long a5 asm("a5") = arg5;
    register long a6 asm("a6") = fid;
    register long a7 asm("a7") = ext;

    asm volatile (
        "ecall"
        : "+r"(a0), "+r"(a1)
        : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
        : "memory"
    );

    ret.error = a0;
    ret.value = a1;
    return ret;
}

void sbi_set_timer(uint64_t stime_value) {
    sbi_call(SBI_EXT_SET_TIMER, 0, stime_value, 0, 0, 0, 0, 0);
}

void sbi_console_putchar(int ch) {
    sbi_call(SBI_EXT_CONSOLE_PUTCHAR, 0, ch, 0, 0, 0, 0, 0);
}

int sbi_console_getchar(void) {
    sbiret_t ret = sbi_call(SBI_EXT_CONSOLE_GETCHAR, 0, 0, 0, 0, 0, 0, 0);
    return ret.error;
}

void sbi_shutdown(void) {
    sbi_call(SBI_EXT_SHUTDOWN, 0, 0, 0, 0, 0, 0, 0);
}
