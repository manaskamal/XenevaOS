/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include <Hal/riscv64_sched.h>
#include <Mm/kmalloc.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <string.h>
#include <aucon.h>
#include <_null.h>
#include "riscv64_lowlevel.h"

/* Thread list pointers */
static AuThread* thread_list_head = NULL;
static AuThread* thread_list_last = NULL;
static AuThread* blocked_thr_head = NULL;
static AuThread* blocked_thr_last = NULL;
static AuThread* trash_thr_head = NULL;
static AuThread* trash_thr_last = NULL;
static AuThread* sleep_thr_head = NULL;
static AuThread* sleep_thr_last = NULL;

/* Current thread and idle thread */
static AuThread* _current_thread = NULL;
static AuThread* _idle_thr = NULL;

/* Thread ID counter */
static uint16_t _thread_id = 0;

/* Scheduler state */
static bool _scheduler_initialized = false;
static uint64_t _scheduler_tick = 0;

/*
 * AuThreadInsert -- Insert a thread to ready list
 * @param new_task -- new thread to insert
 */
static void AuThreadInsert(AuThread* new_task) {
    new_task->next = NULL;
    new_task->prev = NULL;

    if (thread_list_head == NULL) {
        thread_list_last = new_task;
        thread_list_head = new_task;
    } else {
        thread_list_last->next = new_task;
        new_task->prev = thread_list_last;
    }
    thread_list_last = new_task;
}

/*
 * AuThreadDelete -- Remove a thread from ready list
 * @param thread -- thread to remove
 */
static void AuThreadDelete(AuThread* thread) {
    if (thread_list_head == NULL)
        return;

    if (thread == thread_list_head) {
        thread_list_head = thread_list_head->next;
    } else {
        thread->prev->next = thread->next;
    }

    if (thread == thread_list_last) {
        thread_list_last = thread->prev;
    } else {
        thread->next->prev = thread->prev;
    }
}

/*
 * AuThreadInsertBlock -- Insert a thread to blocked list
 * @param new_task -- thread to insert
 */
static void AuThreadInsertBlock(AuThread* new_task) {
    new_task->next = NULL;
    new_task->prev = NULL;

    if (blocked_thr_head == NULL) {
        blocked_thr_last = new_task;
        blocked_thr_head = new_task;
    } else {
        blocked_thr_last->next = new_task;
        new_task->prev = blocked_thr_last;
    }
    blocked_thr_last = new_task;
}

/*
 * AuThreadDeleteBlock -- Remove a thread from blocked list
 * @param thread -- thread to remove
 */
static void AuThreadDeleteBlock(AuThread* thread) {
    if (blocked_thr_head == NULL)
        return;

    if (thread == blocked_thr_head) {
        blocked_thr_head = blocked_thr_head->next;
    } else {
        thread->prev->next = thread->next;
    }

    if (thread == blocked_thr_last) {
        blocked_thr_last = thread->prev;
    } else {
        thread->next->prev = thread->prev;
    }
}

/*
 * AuThreadInsertSleep -- Insert a thread to sleep list
 * @param new_task -- thread to insert
 */
static void AuThreadInsertSleep(AuThread* new_task) {
    new_task->next = NULL;
    new_task->prev = NULL;

    if (sleep_thr_head == NULL) {
        sleep_thr_last = new_task;
        sleep_thr_head = new_task;
    } else {
        sleep_thr_last->next = new_task;
        new_task->prev = sleep_thr_last;
    }
    sleep_thr_last = new_task;
}

/*
 * AuThreadDeleteSleep -- Remove a thread from sleep list
 * @param thread -- thread to remove
 */
static void AuThreadDeleteSleep(AuThread* thread) {
    if (sleep_thr_head == NULL)
        return;

    if (thread == sleep_thr_head) {
        sleep_thr_head = sleep_thr_head->next;
    } else {
        thread->prev->next = thread->next;
    }

    if (thread == sleep_thr_last) {
        sleep_thr_last = thread->prev;
    } else {
        thread->next->prev = thread->prev;
    }
}

/*
 * AuThreadInsertTrash -- Insert a thread to trash list
 * @param new_task -- thread to insert
 */
static void AuThreadInsertTrash(AuThread* new_task) {
    new_task->next = NULL;
    new_task->prev = NULL;

    if (trash_thr_head == NULL) {
        trash_thr_last = new_task;
        trash_thr_head = new_task;
    } else {
        trash_thr_last->next = new_task;
        new_task->prev = trash_thr_last;
    }
    trash_thr_last = new_task;
}

/*
 * AuThreadDeleteTrash -- Remove a thread from trash list
 * @param thread -- thread to remove
 */
static void AuThreadDeleteTrash(AuThread* thread) {
    if (trash_thr_head == NULL)
        return;

    if (thread == trash_thr_head) {
        trash_thr_head = trash_thr_head->next;
    } else {
        thread->prev->next = thread->next;
    }

    if (thread == trash_thr_last) {
        trash_thr_last = thread->prev;
    } else {
        thread->next->prev = thread->prev;
    }
}

/*
 * RISCV64NextThread -- Select the next thread to run
 */
static void RISCV64NextThread() {
    AuThread* thread = _current_thread;
    bool run_idle = false;

    do {
        thread = thread->next;

        if (!thread) {
            thread = thread_list_head;
        }

        /* Skip idle thread in normal selection */
        if (thread == _idle_thr) {
            thread = thread->next;
        }

        if (!thread) {
            run_idle = true;
            break;
        }
    } while (thread->state != THREAD_STATE_READY);

    if (run_idle)
        thread = _idle_thr;

    _current_thread = thread;
}

/*
 * AuHandleSleepThreads -- Process sleeping threads
 */
static void AuHandleSleepThreads() {
    AuThread* sleep_thr;
    for (sleep_thr = sleep_thr_head; sleep_thr != NULL; sleep_thr = sleep_thr->next) {
        if (sleep_thr->sleepQuanta > 0) {
            sleep_thr->sleepQuanta--;
            if (sleep_thr->sleepQuanta == 0) {
                sleep_thr->state = THREAD_STATE_READY;
                AuThreadDeleteSleep(sleep_thr);
                AuThreadInsert(sleep_thr);
            }
        }
    }
}

/* Idle thread entry point */
static void AuIdleThread(uint64_t ctx) {
    AuTextOut("[aurora]: Idle thread running\r\n");
    enable_irq();
    while (1) {
        /* Wait for interrupt */
        __asm__ volatile("wfi");
    }
}

/*
 * AuCreateKernelStack -- Create kernel stack for a thread
 * @param pml -- Page table root
 * @return stack top address
 */
static uint64_t AuCreateKernelStack(uint64_t pml) {
    /* Kernel stack location in RISC-V higher half */
    #define RISCV_KERNEL_STACK_LOCATION  0xFFFFFFC000F00000ULL
    #define RISCV_KERNEL_STACK_SIZE      40960  /* 40KB */
    
    static uint64_t stack_index = 0;
    uint64_t location = RISCV_KERNEL_STACK_LOCATION + (stack_index * RISCV_KERNEL_STACK_SIZE);
    
    for (size_t i = 0; i < RISCV_KERNEL_STACK_SIZE / PAGE_SIZE; i++) {
        uint64_t phys = (uint64_t)AuPmmngrAlloc();
        uint64_t virt_addr = P2V(phys);
        memset((void*)virt_addr, 0, PAGE_SIZE);
        AuMapPage(phys, location + i * PAGE_SIZE, PTE_READ | PTE_WRITE | PTE_VALID);
    }
    
    stack_index++;
    return location + RISCV_KERNEL_STACK_SIZE;
}

/*
 * AuSchedulerInitialise -- Initialize the scheduler
 */
void AuSchedulerInitialise() {
    thread_list_head = NULL;
    thread_list_last = NULL;
    blocked_thr_head = NULL;
    blocked_thr_last = NULL;
    trash_thr_head = NULL;
    trash_thr_last = NULL;
    sleep_thr_head = NULL;
    sleep_thr_last = NULL;
    _thread_id = 0;
    _scheduler_tick = 0;
    
    /* Create idle thread */
    uint64_t* idle_pd = AuCreateVirtualAddressSpace();
    AuThread* idle_ = AuCreateKthread(AuIdleThread, 0, (uint64_t)idle_pd, "Idle");
    _idle_thr = idle_;
    _current_thread = idle_;
    _scheduler_initialized = false;
    
    AuTextOut("[aurora]: Scheduler initialized\r\n");
}

/*
 * AuSchedulerStart -- Start the scheduler
 */
void AuSchedulerStart() {
    if (!_idle_thr) {
        AuTextOut("[aurora]: ERROR - Scheduler not initialized!\r\n");
        return;
    }
    
    _scheduler_initialized = true;
    AuTextOut("[aurora]: Scheduler started\r\n");
    
    /* Switch to idle thread's page table */
    if (_idle_thr->pml) {
        uint64_t satp_val = (8ULL << 60) | (V2P(_idle_thr->pml) >> 12);
        write_satp(satp_val);
        sfence_vma();
    }
    
    /* Jump to idle thread by restoring context from stack */
    /* The stack contains a fake trap frame set up by AuCreateKthread */
    uint64_t sp = _idle_thr->frame.sp;
    
    __asm__ volatile(
        "mv sp, %0\n"       /* Set Stack Pointer to trap frame */
        "ld t0, 128(sp)\n"  /* Load sepc (offset 128) */
        "csrw sepc, t0\n"
        "ld t0, 136(sp)\n"  /* Load sstatus (offset 136) */
        "csrw sstatus, t0\n"
        "ld ra, 0(sp)\n"    /* Load RA (offset 0) */
        /* Note: We should technically restore all regs but for first start 
           of idle/init, only PC/Status/SP matter mostly */
        "addi sp, sp, 256\n" /* Pop frame (256 bytes) */
        "sret\n"             /* Return from S-mode to S-mode (for kernel thread) */
        :: "r"(sp) : "t0", "ra", "memory"
    );
}

/*
 * AuGetCurrentThread -- Return current thread
 */
AU_EXTERN AU_EXPORT AuThread* AuGetCurrentThread() {
    return _current_thread;
}

/*
 * AuCreateKthread -- Create a kernel thread
 * @param entry -- Entry point function
 * @param stack -- Stack pointer (0 to auto-allocate)
 * @param cr3 -- Page table root
 * @param name -- Thread name
 */
/*
 * AuCreateKthread -- Create a kernel thread
 * @param entry -- Entry point function
 * @param stack -- Stack pointer (0 to auto-allocate)
 * @param cr3 -- Page table root
 * @param name -- Thread name
 */
AU_EXTERN AU_EXPORT AuThread* AuCreateKthread(void(*entry) (uint64_t), uint64_t stack, uint64_t cr3, char *name) {
    AuThread* t = (AuThread*)kmalloc(sizeof(AuThread));
    memset(t, 0, sizeof(AuThread));
    
    strncpy(t->name, name, 15);
    t->name[15] = '\0';
    
    t->pml = cr3;
    
    /* Create kernel stack if not provided */
    uint64_t kstack;
    if (stack == 0) {
        kstack = AuCreateKernelStack(cr3);
    } else {
        kstack = stack;
    }
    t->originalKSp = kstack;
    
    /* Prepare stack for first run (fake trap frame) 
     * Layout must match riscv64_lowlevel.s: riscv64_trap_vector_entry
     * Size: 256 bytes
     */
    uint64_t sp = kstack;
    sp -= 256;          /* Allocate trap frame */
    sp &= ~0xF;         /* Align 16 bytes */
    
    uint64_t* frame = (uint64_t*)sp;
    memset(frame, 0, 256);
    
    /* Offsets (bytes / 8) from asm:
     * 0: ra
     * 8: t0
     * ...
     * 128: sepc
     * 136: sstatus
     * ...
     */
    
    /* Set Entry Point */
    frame[0] = (uint64_t)entry;     /* ra: return address (if func returns) */
    frame[16] = (uint64_t)entry;    /* sepc: where sret jumps to (offset 128) */
    
    /* Set Status */
    /* SSTATUS: SPIE=1 (bit 5), SPP=1 (bit 8) -> 0x120 */
    frame[17] = 0x120;              /* sstatus (offset 136) */
    
    /* Save this stack pointer as the thread's saved context */
    /* We use t->frame.sp to store the Top of Stack of the saved context */
    t->frame.sp = sp;
    
    t->id = _thread_id++;
    t->state = THREAD_STATE_READY;
    t->threadType = THREAD_LEVEL_KERNEL;
    t->first_run = 0;
    
    AuThreadInsert(t);
    return t;
}

/*
 * AuThreadFindByID -- Find a thread by ID in ready queue
 * @param id -- Thread ID
 */
AU_EXTERN AU_EXPORT AuThread* AuThreadFindByID(uint16_t id) {
    AuThread* thr;
    for (thr = thread_list_head; thr != NULL; thr = thr->next) {
        if (thr->id == id)
            return thr;
    }
    return NULL;
}

/*
 * AuThreadFindByIDBlockList -- Find a thread by ID in blocked queue
 * @param id -- Thread ID
 */
AU_EXTERN AU_EXPORT AuThread* AuThreadFindByIDBlockList(uint16_t id) {
    AuThread* thr;
    for (thr = blocked_thr_head; thr != NULL; thr = thr->next) {
        if (thr->id == id)
            return thr;
    }
    return NULL;
}

/*
 * AuBlockThread -- Block a running thread
 * @param thread -- Thread to block
 */
AU_EXTERN AU_EXPORT void AuBlockThread(AuThread *thread) {
    thread->state = THREAD_STATE_BLOCKED;
    AuThreadDelete(thread);
    AuThreadInsertBlock(thread);
}

/*
 * AuUnblockThread -- Unblock a thread
 * @param thread -- Thread to unblock
 */
AU_EXTERN AU_EXPORT void AuUnblockThread(AuThread *t) {
    t->state = THREAD_STATE_READY;
    bool found = false;
    AuThread* thr;
    
    for (thr = blocked_thr_head; thr != NULL; thr = thr->next) {
        if (thr == t) {
            AuThreadDeleteBlock(thr);
            found = true;
            break;
        }
    }
    
    if (found)
        AuThreadInsert(t);
}

/*
 * AuSleepThread -- Put a thread to sleep
 * @param thread -- Thread to sleep
 * @param ms -- Milliseconds to sleep
 */
AU_EXTERN AU_EXPORT void AuSleepThread(AuThread *thread, uint64_t ms) {
    thread->state = THREAD_STATE_SLEEP;
    thread->sleepQuanta = ms;
    AuThreadDelete(thread);
    AuThreadInsertSleep(thread);
}

/*
 * AuForceScheduler -- Force a context switch
 */
AU_EXTERN AU_EXPORT void AuForceScheduler() {
    if (!_scheduler_initialized)
        return;
    
    /* Trigger S-mode ecall to enter trap handler and reschedule */
    __asm__ volatile("ecall");
}

/*
 * AuScheduleThread -- Schedule a specific thread
 * @param stack_frame -- Current stack pointer from trap
 * @return New stack pointer to switch to
 */
AU_EXTERN AU_EXPORT uint64_t AuScheduleThread(uint64_t stack_frame) {
    if (!_scheduler_initialized)
        return stack_frame;
    
    /* Save current context if valid */
    if (stack_frame && _current_thread) {
        _current_thread->frame.sp = stack_frame;
    }
    
    _scheduler_tick++;
    AuHandleSleepThreads();
    RISCV64NextThread();
    
    /* Switch page tables if needed */
    if (_current_thread->pml) {
        uint64_t satp_val = (8ULL << 60) | (V2P(_current_thread->pml) >> 12);
        write_satp(satp_val);
        sfence_vma();
    }
    
    return _current_thread->frame.sp;
}

/*
 * AuIsSchedulerInitialised -- Check if scheduler is initialized
 */
AU_EXTERN AU_EXPORT bool AuIsSchedulerInitialised() {
    return _scheduler_initialized;
}

/*
 * AuGetSystemTimerTick -- Return current system timer tick
 */
AU_EXTERN AU_EXPORT uint64_t AuGetSystemTimerTick() {
    return _scheduler_tick;
}

/*
 * AuThreadMoveToTrash -- Move a thread to trash list for cleanup
 * @param t -- Thread to trash
 */
void AuThreadMoveToTrash(AuThread* t) {
    if (!t)
        return;
    
    t->state = THREAD_STATE_KILLABLE;
    
    /* Search and remove from ready queue */
    AuThread* thr;
    for (thr = thread_list_head; thr != NULL; thr = thr->next) {
        if (thr == t) {
            AuThreadDelete(t);
            break;
        }
    }
    
    /* Search and remove from blocked queue */
    for (thr = blocked_thr_head; thr != NULL; thr = thr->next) {
        if (thr == t) {
            AuThreadDeleteBlock(t);
            break;
        }
    }
    
    /* Insert to trash */
    AuThreadInsertTrash(t);
}

/*
 * AuThreadCleanTrash -- Clean up trashed threads
 * @param t -- Thread to clean (unused, cleans all)
 */
void AuThreadCleanTrash(AuThread* t) {
    /* TODO: Implement proper cleanup */
}
