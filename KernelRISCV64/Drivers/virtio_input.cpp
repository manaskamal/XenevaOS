/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

#include <stdint.h>
#include <aurora.h>
#include "../../Drivers/Virtio/virtio.h"
#include "../Hal/riscv64_hal.h"
#include "../Hal/riscv64_lowlevel.h"
#include <dtb.h>
#include <aucon.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <string.h>
#include <_null.h>
#include <Fs/Dev/devinput.h>

#ifndef OFFSETOF
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif

extern "C" void AuRegisterPLICIrq(uint32_t irq, void (*handler)(uint32_t));

#define VIRTIO_INPUT_CFG_ID_NAME     0x01
#define VIRTIO_INPUT_CFG_ID_SERIAL   0x02

/* VirtIO MMIO Device IDs */
#define VIRTIO_MMIO_ID_NET      1
#define VIRTIO_MMIO_ID_BLOCK    2
#define VIRTIO_MMIO_ID_CONSOLE  3
#define VIRTIO_MMIO_ID_RNG      4
#define VIRTIO_MMIO_ID_BALLOON  5
#define VIRTIO_MMIO_ID_IOMEM    6
#define VIRTIO_MMIO_ID_RPMSG    7
#define VIRTIO_MMIO_ID_SCSI     8
#define VIRTIO_MMIO_ID_9P       9
#define VIRTIO_MMIO_ID_RPROC_SERIAL 11
#define VIRTIO_MMIO_ID_CAIF     12
#define VIRTIO_MMIO_ID_GPU      16
#define VIRTIO_MMIO_ID_INPUT    18

/* Key Map (ported from virtiokbd.c) */
static const uint8_t ext_key_map[256] = {
	[0x63] = 0x37, //print screen
	[0x66] = 0x47, //home
	[0x67] = 0x48, //up
	[0x68] = 0x49, //page up
	[0x6c] = 0x50, //down
	[0x69] = 0x4B, //left
	[0x6a] = 0x4D, //right
	[0x6b] = 0x4F, //end
	[0x6d] = 0x51, //page down
	[0x7d] = 0x5b, //left super
};

struct VirtioInputEvent {
    uint16_t type;
    uint16_t code;
    uint32_t value;
};

/* Internal Driver State */
struct VirtioInputDev {
    VirtioMMIOHeader* mmio;
    struct VirtioQueue* eventQ;
    struct VirtioQueue* statusQ;
    struct VirtioInputEvent* eventBuf;
    uint32_t queueSize;
    uint16_t usedIdx;
};

static struct VirtioInputDev* inputDev = NULL;

/* 
 * AuVirtioInputHandler -- PLIC Interrupt Handler
 */
void AuVirtioInputHandler(uint32_t irq) {
    if (!inputDev) return;
    
    // Ack interrupt in MMIO? (Reading InterruptStatus and writing InterruptACK)
    uint32_t status = inputDev->mmio->InterruptStatus;
    inputDev->mmio->InterruptACK = status;
    
    // Process Used Ring
    struct VirtioQueue* q = inputDev->eventQ;
    uint16_t currentUsed = q->used.index;
    
    while (inputDev->usedIdx != currentUsed) {
        uint16_t id = q->used.ring[inputDev->usedIdx % inputDev->queueSize].index;
        // id should correspond to the buffer index we put in Avail
        
        struct VirtioInputEvent* evt = &inputDev->eventBuf[id];
        
        // Ported Logic from virtiokbd.c
        if (evt->type == 1) { // EV_KEY
            if (evt->code < 0x49) {
                uint8_t scancode = evt->code;
                if (evt->value == 0) { // Release
                    scancode |= 0x80;
                }
                
                AuInputMessage msg;
                memset(&msg, 0, sizeof(AuInputMessage));
                msg.type = AU_INPUT_KEYBOARD;
                msg.code = scancode & 0xFF;
                AuDevWriteKybrd(&msg);
            }
            else if (ext_key_map[evt->code]) {
                uint32_t scancode = (0xE0 & 0xFF) << 16 | (ext_key_map[evt->code] & 0xFF) << 8 | (((evt->value == 0) ? 0x80 : 0) & 0xFF);
                AuInputMessage msg;
                memset(&msg, 0, sizeof(AuInputMessage));
                msg.type = AU_INPUT_KEYBOARD;
                msg.code = scancode;
                AuDevWriteKybrd(&msg);
            }
        }
        
        // Repopulate Avail Ring (Add it back)
        // Usually we should check if we need to advance Avail
        // But here we just recycle the buffer.
        // virtiokbd.c logic: "queue->available.index++"
        
        q->available.ring[q->available.index % inputDev->queueSize] = id;
        q->available.index++;
        
        inputDev->usedIdx++;
    }
    
    // Notify device
    inputDev->mmio->QueueNotify = 0; // Queue 0 (Event)
}

/*
 * VirtioInputInitDevice -- Initialize found MMIO device
 */
void VirtioInputInitDevice(uint64_t base, uint32_t irq) {
    AuTextOut("[virtio-input]: Found MMIO Input at %x (IRQ %d)\n", base, irq);
    
    VirtioMMIOHeader* mmio = (VirtioMMIOHeader*)base;
    
    if (mmio->MagicValue != VIRTIO_MAGIC_VALUE) {
        AuTextOut("[virtio-input]: Invalid Magic %x\n", mmio->MagicValue);
        return;
    }
    
    if (mmio->DeviceID != VIRTIO_MMIO_ID_INPUT) {
        return;
    }
    
    AuTextOut("[virtio-input]: Version=%d\n", mmio->Version);

    if (inputDev) return; // Only support one keyboard for now
    
    inputDev = (struct VirtioInputDev*)kmalloc(sizeof(struct VirtioInputDev));
    memset(inputDev, 0, sizeof(struct VirtioInputDev));
    inputDev->mmio = mmio;

    // 1. Reset
    mmio->Status = 0;
    sfence_vma();
    
    // 2. Status: Acknowledge (explicit write, not read-modify-write via |=)
    mmio->Status = VIRTIO_STATUS_ACKNOWLEDGE;
    sfence_vma();
    
    // 3. Status: Driver
    mmio->Status = mmio->Status | VIRTIO_STATUS_DRIVER;
    sfence_vma();
    
    // 4. Features — accept what device offers
    mmio->DeviceFeaturesSel = 0;
    sfence_vma();
    uint32_t features = mmio->DeviceFeatures;
    mmio->DriverFeaturesSel = 0;
    mmio->DriverFeatures = features;
    sfence_vma();
    
    mmio->Status = mmio->Status | VIRTIO_STATUS_FEATURES_OK;
    sfence_vma();
    
    // 5. Setup Queues — v1 format (GuestPageSize + QueuePFN)
    // Must set GuestPageSize before any queue setup
    mmio->GuestPageSize = 4096;
    sfence_vma();
    
    // Queue 0: EventQ
    mmio->QueueSel = 0;
    sfence_vma();
    
    uint32_t qMax = mmio->QueueNumMax;
    if (qMax == 0) {
        AuTextOut("[virtio-input]: Queue 0 not available\n");
        return;
    }
    AuTextOut("[virtio-input]: QueueNumMax=%d\n", qMax);
    
    // Cap to 64 to match our fixed VirtioQueue struct
    if (qMax > 64) qMax = 64;
    inputDev->queueSize = qMax;
    mmio->QueueNum = qMax;
    sfence_vma();
    
    // Allocate and zero queue memory
    uint64_t qPhys = (uint64_t)AuPmmngrAlloc();
    memset((void*)P2V(qPhys), 0, 4096);
    
    // v1: Set alignment and PFN
    mmio->QueueAlign = 4096;
    sfence_vma();
    mmio->QueuePFN = (uint32_t)(qPhys / 4096);
    sfence_vma();
    
    AuTextOut("[virtio-input]: Queue PFN=0x%x\n", (uint32_t)(qPhys / 4096));
    inputDev->eventQ = (struct VirtioQueue*)P2V(qPhys);
    
    // 6. Fill Event Queue with buffers
    uint64_t bufPhys = (uint64_t)AuPmmngrAlloc();
    inputDev->eventBuf = (struct VirtioInputEvent*)P2V(bufPhys);
    
    for (int i = 0; i < (int)qMax; i++) {
        inputDev->eventQ->buffers[i].Addr = bufPhys + i * sizeof(struct VirtioInputEvent);
        inputDev->eventQ->buffers[i].Length = sizeof(struct VirtioInputEvent);
        inputDev->eventQ->buffers[i].Flags = 2; // Write (Device writes to us)
        inputDev->eventQ->buffers[i].Next = 0;
        
        inputDev->eventQ->available.ring[i] = i;
    }
    inputDev->eventQ->available.index = qMax;
    
    // 7. Status: Driver OK
    mmio->Status = mmio->Status | VIRTIO_STATUS_DRIVER_OK;
    sfence_vma();
    
    AuRegisterPLICIrq(irq, AuVirtioInputHandler);
    
    AuTextOut("[virtio-input]: Registered IRQ %d. Waiting for interrupts.\r\n", irq);
}

/*
 * AuVirtIOInputProbe -- Callback for DTB Scan
 */
void AuVirtIOInputProbe(uint32_t* node, void* arg) {
    AuTextOut("[virtio-input]: Probing node...\r\n");
    // Hardcoding Cells: QEMU RISC-V Virt uses 2 addr / 2 size for MMIO buses.
    // Searching the Device Tree backwards from inside 'soc' causes a parser crash.
    uint32_t addrCells = 2;
    uint32_t sizeCells = 2;
    
    AuTextOut("[virtio-input]: addrCells: %x, sizeCells: %x\r\n", addrCells, sizeCells);
    
    uint64_t reg = AuDeviceTreeGetRegAddress(node, addrCells);
    uint64_t size = AuDeviceTreeGetRegSize(node, addrCells, sizeCells);
    
    AuTextOut("[virtio-input]: Reg: %x, Size: %x\r\n", reg, size);
    if (reg == 0) return;
    
    // Virtual mapping requires page-aligned start addresses
    uint64_t aligned_reg = reg & ~0xFFFULL;
    uint64_t offset = reg & 0xFFFULL;
    
    AuTextOut("[virtio-input]: Aligned Reg: %x, offset: %x\r\n", aligned_reg, offset);
    
    // Map MMIO
    uint64_t mmio_base = (uint64_t)AuMapMMIO(aligned_reg, ((size + offset) / 4096) + 1);
    uint64_t mmio = mmio_base + offset;
    
    AuTextOut("[virtio-input]: Mapped MMIO Virtual: %x\r\n", mmio);
    
    // Check Magic and ID using 32-bit hardware reads
    volatile uint32_t* mmio_regs = (volatile uint32_t*)mmio;
    uint32_t magic = mmio_regs[0];
    uint32_t dev_id = mmio_regs[2];
    
    AuTextOut("[virtio-input]: Reading Magic... Magic: %x, DevID: %d\r\n", magic, dev_id);
    if (magic == VIRTIO_MAGIC_VALUE) {
        if (dev_id == VIRTIO_MMIO_ID_INPUT) {
             uint32_t irq = AuDeviceTreeGetInterrupt(node);
             VirtioInputInitDevice(mmio, irq);
        }
    }
}

/*
 * AuVirtIOInputInitialize -- Entry point called from init.cpp
 */
void AuVirtIOInputInitialize() {
    AuTextOut("[virtio-input]: Scanning for VirtIO MMIO devices...\r\n");
    AuDeviceTreeScan("virtio_mmio@", AuVirtIOInputProbe, NULL);
}
