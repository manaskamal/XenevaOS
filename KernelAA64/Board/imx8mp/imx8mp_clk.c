/**
* @file imx8mp_clk.c
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

static uint64_t _ccm_base;

/**
 * imx8mp_hdmi_ccm_init -- initialize hdmi root clocks
 */
void imx8mp_hdmi_ccm_init() {
	/** start HDMI APB + AXI clock root */
	uint32_t setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_MEDIA_AXI_SYS_PLL2_500M) |
		TARGET_ROOT_PRE(0) | TARGET_ROOT_POST(0);
	imx8mp_ccm_write(HDMI_AXI_CLK_ROOT, 0x00, setval);
	setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_MEDIA_APB_SYS_PLL1_133M) | TARGET_ROOT_PRE(0) |
		TARGET_ROOT_POST(0);
	imx8mp_ccm_write(HDMI_APB_CLK_ROOT, 0x00, setval);

	setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_HDMI_FDCC_SYS_PLL1_266M) | TARGET_ROOT_PRE(0) |
		TARGET_ROOT_POST(0);
	imx8mp_ccm_write(HDMI_REF_266M_ROOT, 0x00, setval);

	setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_HDMI_24M_OSC_24M) | TARGET_ROOT_PRE(0) |
		TARGET_ROOT_POST(0);
	imx8mp_ccm_write(HDMI_24M_ROOT, 0x00, setval);

	setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_HDMI_FDCC_SYS_PLL1_266M) | TARGET_ROOT_PRE(0) |
		TARGET_ROOT_POST(0);
	imx8mp_ccm_write(HDMI_FDCC_TST_CLK_ROOT, 0x00, setval);

}

/**
 * imx8mp_lcdif_ccm_enable -- initialize lcdif ccm clocks
 */
void imx8mp_lcdif_ccm_init() {
	uint32_t setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_MEDIA_AXI_SYS_PLL2_500M) |
		TARGET_ROOT_PRE(0) | TARGET_ROOT_POST(0);
	imx8mp_ccm_write(MEDIA_AXI_CLK_ROOT, 0x00, setval);

	AuTextOut("media axi clk root enabled \r\n");
	setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_MEDIA_APB_SYS_PLL1_133M) |
		TARGET_ROOT_PRE(0) | TARGET_ROOT_POST(0);
	imx8mp_ccm_write(MEDIA_APB_CLK_ROOT, 0x00, setval);

	AuTextOut("media apb clk root enabled \r\n");

	/*setval = TARGET_ROOT_ENABLE | TARGET_ROOT_MUX(MUX_DISP2_PIX_VIDEO_PLL1) |
		TARGET_ROOT_PRE(0) | TARGET_ROOT_POST(0);
	imx8mp_ccm_write(MEDIA_DISP2_CLK_ROOT, 0x00, setval);*/
}

/**
 *imx8mp_void_ccm_init -- map the ccm module
 */
void imx8mp_ccm_init() {
	AuTextOut("[imx8mp_board]: initializing clock control module (ccm) \r\n");
	_ccm_base = (uint64_t)AuMapMMIO(CCM_BASE, 16);
	
	/** by default let's only enable HDMI + LCDIF, because we need 
	 * framebuffer output :) 
	 */
	imx8mp_hdmi_ccm_init();
	AuTextOut("[imx8mp_board]: hdmi apb and axi clock root enabled \r\n");
	imx8mp_lcdif_ccm_init();

	/** todo try setting the clr bits also */
	AuTextOut("[imx8mp_board]: ldcif clock root enabled \r\n");
	AuTextOut("[imx8mp_board]: clock control module (ccm) initialized \r\n");
}

/**
 * imx8mp_ccm_write -- write value to clock indexed register
 */
void imx8mp_ccm_write(uint32_t clk_root_idx, int offset, uint32_t value) {
	(*(volatile uint32_t*)(CCM_ROOT_REG(_ccm_base, clk_root_idx) + offset)) = value;
	dsb_sy_barrier();
	isb_flush();
}

#endif

