/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#ifndef __NVME_H__
#define __NVME_H__

#include <stdint.h>
#include <aurora.h>

typedef struct _nvme_dev_ {
	uint64_t mmiobase;
	uint8_t majorVersion;
	uint8_t minorVersion;
	uint16_t maxQueueEntries;
}NVMeDev;

#define NVME_REGISTER_CAP 0x00
#define NVME_REGISTER_VS  0x08 //version
#define NVME_REGISTER_INTMS 0x0C //interrupt mask set
#define NVME_REGISTER_INTMC 0x10 //interrupt mask clear
#define NVME_REGISTER_CC 0x14  //controller configuration
#define NVME_REGISTER_CSTS 0x1C //controller status
#define NVME_REGISTER_AQA 0x24 //Admin queue attributes
#define NVME_REGISTER_ACQ 0x30 //Admin completion queue

#define NVME_CC_DISABLE   0
#define NVME_CC_EN        0x1
#define NVME_CC_EN_MASK   0x1
#define NVME_CC_CSNVME    0x0
#define NVME_CC_CSS_MASK  0x70
#define NVME_CC_MPS_MASK  0x780
#define NVME_CC_MPS_SHIFT 7
#define NVME_CC_AMS_ROUNDROBIN  0
#define NVME_CC_AMS_WIGHTED     (1<<11)
#define NVME_CC_AMS_MASK        (0x7 << 11)

#define NVME_CAP_MPS_MASK 0xfU
#define NVME_CAP_MPSMAX(x) ((x >> 52) & NVME_CAP_MPS_MASK)
#define NVME_CAP_MPSMIN(x) ((x >> 48) & NVME_CAP_MPS_MASK)

#define NVME_CAP_MQES_MASK 0xffffU
#define NVME_CAP_MQES(x) ((x) & NVME_CAP_MQES_MASK)


#endif