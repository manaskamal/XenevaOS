/**
* @file imx8mp_gpc.c
*
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#ifdef __TARGET_BOARD_IMX8MP_VERDIN_DAHLIA__ || (__TARGET_BOARD_IMX8MP_SOC__)

#include <Board/imx8mp/imx8mp_clk.h>
#include <Drivers/uart.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <aucon.h>

static uint64_t _gpc_base;

#define GPC_BASE 0x303A0000UL
#define GPC_PGC_CPU_MAPPING 0x1CC
#define GPC_PU_PGC_SW_PUP_REQ 0x0F8
#define GPC_PGC_CTRL(n) (0x800 + n * 0x40)
#define GPC_PGC_SR(n)   (GPC_PGC_CTRL(n) + 0x0C)

#define IMX8MP_PGC_MEDIAMIX 22
#define IMX8MP_MEDIAMIX_A53_DOMAIN (1ul << 12)

#define GPC_WRITE(base, offset, val)   (*(volatile uint32_t*)(base + offset) = val)
#define GPC_READ(base, offset)  (*(volatile uint32_t*)(base + offset))

void imx8mp_gpc_init() {
	_gpc_base = (uint64_t)AuMapMMIO(GPC_BASE, 2);
	GPC_WRITE(_gpc_base, GPC_PGC_CTRL(IMX8MP_PGC_MEDIAMIX), 0x1);
	AuTextOut("Base offset : %x \r\n", (GPC_BASE + GPC_PGC_CTRL(IMX8MP_PGC_MEDIAMIX)));

	GPC_WRITE(_gpc_base, GPC_PGC_CPU_MAPPING, IMX8MP_MEDIAMIX_A53_DOMAIN);

	GPC_WRITE(_gpc_base, GPC_PU_PGC_SW_PUP_REQ, (1ULL << (IMX8MP_PGC_MEDIAMIX - 8)));

	while (GPC_READ(_gpc_base, GPC_PGC_SR(IMX8MP_PGC_MEDIAMIX)) & 0x2)
		;
	AuTextOut("[imx8mp_board]: gpc powered up mediamix \r\n");
}
#endif