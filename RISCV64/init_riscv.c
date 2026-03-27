/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2023-2026, Manas Kamal Choudhury
 * All rights reserved.
 *
 * init_riscv.c -- XenevaOS RISC-V user-space init process
 *
 * This is the first user-space program that runs on RISC-V.
 * It is loaded by the kernel from /init.exe in the InitRD.
 *
 * Responsibilities:
 *  - Print a startup banner via kernel debug print (syscall 1)
 *  - Launch the Deodhai compositor
 *  - Become the kernel's dead-process reaper
 **/

/*
 * NOTE: This is a statically linked, standalone binary with no libc.
 * All kernel calls go through the _callbase_riscv64.s stubs.
 */

/* Syscall stubs — provided by _callbase_riscv64.s */
extern int _KePrint(const char* fmt, ...);
extern int _KeGetProcessID(void);
extern int _KeGetThreadID(void);
extern int _KeCreateProcess(int parent_id, const char* name);
extern int _KeProcessLoadExec(int proc_id, const char* filename, int argc, char** argv);
extern int _KePauseThread(void);
extern int _KeProcessExit(void);
extern int _KeProcessSleep(unsigned long ms);
extern int _KeProcessWaitForTermination(int pid);
extern int _KeOpenFile(const char* path, int mode);
extern int _KeCloseFile(int fd);

#define FILE_OPEN_READ_ONLY  0x1

/**
 * init_launch -- launch a child process from filename
 * Returns the process id or -1 on failure
 */
static int init_launch(const char* name, const char* filename) {
    int proc = _KeCreateProcess(0, name);
    if (proc < 0) {
        _KePrint("[init] Failed to create process slot for %s\n", name);
        return -1;
    }
    int ret = _KeProcessLoadExec(proc, filename, 0, (char**)0);
    if (ret < 0) {
        _KePrint("[init] Failed to load %s into process %d\n", filename, proc);
        return -1;
    }
    _KePrint("[init] Launched %s (pid=%d)\n", filename, proc);
    return proc;
}

/**
 * main -- init process entry point
 * Called by crt0_riscv64.s with argc/argv from kernel
 */
void main(int argc, char* argv[]) {
    int pid = _KeGetProcessID();
    int tid = _KeGetThreadID();

    _KePrint("\n");
    _KePrint("==============================================\n");
    _KePrint("  XenevaOS RISC-V Init Process\n");
    _KePrint("  Build: " __DATE__ " " __TIME__ "\n");
    _KePrint("  PID=%d  TID=%d\n", pid, tid);
    _KePrint("==============================================\n");
    _KePrint("\n");

    /*
     * Launch background services and compositor.
     * Each call blocks until the process slot is created and the
     * binary is scheduled; the process itself runs concurrently.
     */

    /* Brief delay to let earlier kernel output flush */
    _KeProcessSleep(10);

    /* Launch Deodhai compositor */
    _KePrint("[init] Starting Deodhai compositor...\n");
    init_launch("deodhai", "/deodhai.exe");
    _KeProcessSleep(100);

    /* Launch session manager / launcher */
    _KePrint("[init] Starting XE Launcher...\n");
    init_launch("xelnch", "/xelnch.exe");
    _KeProcessSleep(100);

    /*
     * Become the reaper: wait for any child to terminate.
     * This keeps the init process alive as PID 1.
     */
    _KePrint("[init] Init loop — becoming process reaper\n");
    while (1) {
        _KeProcessWaitForTermination(-1);
    }
}
