#ifndef _XE_LOW_LEVEL_H_
#define _XE_LOW_LEVEL_H_

#include <stdint.h>
#include "xnldr.h"

extern "C" uint64_t _getCurrentEL();
extern "C" uint64_t read_satp();
extern "C" void write_satp(uint64_t val);
extern "C" void tlb_flush_all();
extern "C" void callKernel(XEBootInfo* bootinfo, uint64_t stackbase, uint64_t stacksize, void* entry);
extern "C" void _hang();

#endif
