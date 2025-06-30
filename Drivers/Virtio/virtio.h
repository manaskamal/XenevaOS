/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <stdint.h>

#define VIRTIO_MAGIC_VALUE 0x74726976
#define VIRTIO_VENDOR_ID_QEMU 0x554D4551

#pragma pack(push,1)
typedef struct _virtio_mmio_ {
	volatile uint32_t MagicValue;
	volatile uint32_t Version;
	volatile uint32_t DeviceID;
	volatile uint32_t VendorID;
	volatile uint32_t DeviceFeatures;
	volatile uint32_t DeviceFeaturesSel;
	volatile uint32_t reserved_018;
	volatile uint32_t reserved_01C;
	volatile uint32_t DriverFeatures;
	volatile uint32_t DriverFeaturesSel;
	volatile uint32_t QueueSel;
	volatile uint32_t QueueNumMax;
	volatile uint32_t QueueNum;
	volatile uint32_t QueueAlign;
	volatile uint32_t QueuePFN;
	volatile uint32_t reserved_03C;
	volatile uint32_t QueueReady;
	volatile uint32_t reserved_044;
	volatile uint32_t QueueNotify;
	volatile uint32_t reserved_04C;
	volatile uint32_t InterruptStatus;
	volatile uint32_t InterruptACK;
	volatile uint32_t Status;
	volatile uint32_t reserved_05C;
	volatile uint32_t QueueDescLow;
	volatile uint32_t QueueDescHigh;
	volatile uint32_t QueueDriverLow;
	volatile uint32_t QueueDriverHigh;
	volatile uint32_t QueueDeviceLow;
	volatile uint32_t QueueDeviceHigh;
	volatile uint32_t ConfigGeneration;
	volatile uint32_t reserved_07C;
	volatile uint8_t DeviceConfig[0x200 - 0x80];
}VirtioHeader;
#pragma pack(pop)

#endif