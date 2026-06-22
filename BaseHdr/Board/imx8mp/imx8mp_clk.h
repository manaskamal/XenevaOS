/**
* @file imx8mp_clk.h
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

#ifndef __IMX8MP_CLK_H__
#define __IMX8MP_CLK_H__

#include <aurora.h>
/**
 * Clock Control Module regs
 */

// as per nxp ref
#define CCM_BASE 0x30380000

/** clock roots **/
typedef enum _clock_root_select_ {
	ARM_A53_CLK_ROOT = 0,
	ARM_M7_CLK_ROOT,
	ML_CLK_ROOT,
	GPU3D_CORE_CLK_ROOT,
	GPU3D_SHADER_CLK_ROOT,
	GPU2D_CLK_ROOT,
	AUDIO_AXI_CLK_ROOT,
	HSIO_AXI_CLK_ROOT,
	MEDIA_ISP_CLK_ROOT,
	MAIN_AXI_CLK_ROOT = 16,
	ENET_AXI_CLK_ROOT,
	NAND_USDHC_BUS_CLK_ROOT,
	VPU_BUS_CLK_ROOT,
	MEDIA_AXI_CLK_ROOT,
	MEDIA_APB_CLK_ROOT,
	HDMI_APB_CLK_ROOT,
	HDMI_AXI_CLK_ROOT,
	GPU_AXI_CLK_ROOT,
	GPU_AHB_CLK_ROOT,
	NOC_CLK_ROOT,
	NOC_IO_CLK_ROOT,
	ML_AXI_CLK_ROOT,
	ML_AHB_CLK_ROOT,
	AHB_CLK_ROOT = 32,
	IPG_CLK_ROOT,
	AUDIO_AHB_CLK_ROOT,
	MEDIA_DISP2_CLK_ROOT = 38,
	DRAM_CLK_ROOT_SEL = 48,
	ARM_A53_CLK_ROOT_SEL,
	DRAM_ALT_CLK_ROOT = 64,
	DRAM_APB_CLK_ROOT, 
	VPU_G1_CLK_ROOT,
	VPU_G2_CLK_ROOT,
	CAN1_CLK_ROOT,
	CAN2_CLK_ROOT,
	MEMREPAIR_CLK_ROOT = 70,
	PCIE_PHY_CLK_ROOT,
	PCIE_AUX_CLK_ROOT,
	I2C5_CLK_ROOT,
	I2C6_CLK_ROOT,
	SAI1_CLK_ROOT,
	HDMI_FDCC_TST_CLK_ROOT = 118,
	HDMI_24M_ROOT,
	HDMI_REF_266M_ROOT,
	/* incomplete */
}imx8mp_clock_root_select;
/**
 * formula to get specific peripheral clock source
 */
#define CCM_ROOT_REG(base, root_idx)  (base + 0x8000 + (root_idx * 0x80))
#define CCM_TARGET_ROOT(base, root_idx) CCM_ROOT_REG(base, root_idx)
#define CCM_TARGET_ROOT_SET(base, root_idx) (CCM_ROOT_REG(base, root_idx) + 0x04)
#define CCM_TARGET_ROOT_CLR(base, root_idx) (CCM_ROOT_REG(base, root_idx) + 0x08)
#define CCM_TARGET_ROOT_TOG(base, root_idx) (CCM_ROOT_REG(base, root_idx) + 0x0C)

#define TARGET_ROOT_ENABLE (1u << 28)
#define TARGET_ROOT_MUX(x)  (((x) & 0x7u) << 24)
#define TARGET_ROOT_PRE(x)  (((x) & 0x7u) << 16)
#define TARGET_ROOT_POST(x) (((x) & 0x3Fu) << 0)


#define MUX_MEDIA_AXI_SYS_PLL2_500M    7
#define MUX_MEDIA_APB_SYS_PLL1_133M    7
#define MUX_HDMI_REF_266M_SYS_PLL1_266 4
#define MUX_HDMI_24M_OSC_24M           0
#define MUX_HDMI_FDCC_SYS_PLL1_266M    1
#define MUX_DISP2_PIX_VIDEO_PLL1       1


/**
 * @brief imx8mp_void_ccm_init -- map the ccm module
 */
extern void imx8mp_ccm_init();

 /**
  * @brief imx8mp_ccm_write -- write value to clock indexed register
  */
AU_EXTERN AU_EXPORT void imx8mp_ccm_write(uint32_t clk_root_idx,int offset, uint32_t value);

#endif