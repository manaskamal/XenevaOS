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

#if defined(__TARGET_BOARD_IMX8MP_VERDIN_DAHLIA__) || defined(__TARGET_BOARD_IMX8MP_SOC__)

#include <Board/imx8mp/imx8mp_clk.h>
#include <Drivers/uart.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Log/klog.h>
#include <aucon.h>
#include <_null.h>
#include <Board/imx8mp/imx8mp_pll.h>
#include <dtb.h>

static uint64_t _ccm_base;

typedef struct _clk_node_ {
	char* name;
	uint32_t(*recalc_rate)(struct _clk_node_* self);
	int num_parent;
	struct _clk_node_** parent;
	uint32_t reg_offset;
	uint32_t pre_podf;
	uint32_t post_podf;
	int is_composite;
	uint32_t anatop_base;
	bool _pll_read;
}imx8mp_clk;

static imx8mp_clk _clk_node[100];
static int _clk_node_count = 0;

static imx8mp_clk* _imx8mp_clk_alloc(const char* name) {
	imx8mp_clk* n = &_clk_node[_clk_node_count++];
	n->name = name;
	n->parent = NULL;
	n->is_composite = 0;
	return n;
}

static uint32_t _imx8mp_fixed_recalc(imx8mp_clk* clk) {
	return clk->pre_podf;
}

static imx8mp_clk* imx8mp_clk_fixed(const char* name, uint32_t fixed_hz) {
	imx8mp_clk* n = _imx8mp_clk_alloc(name);
	n->recalc_rate = _imx8mp_fixed_recalc;
	n->pre_podf = fixed_hz;
	return n;
}

static imx8mp_clk* imx8mp_clk_composite(const char* name, imx8mp_clk** parent, int n_parents) {
	imx8mp_clk* n = _imx8mp_clk_alloc(name);
	n->recalc_rate = 0;
	n->num_parent = n_parents;
	n->parent = parent;
	n->is_composite = 1;
	n->pre_podf = 0;
	n->post_podf = 0;
	n->anatop_base = 0;
	n->_pll_read = 0;
	return n;
}


static imx8mp_clk* imx8mp_clk_pll(const char* name, imx8mp_clk** parent, int n_parents, uint32_t anatop) {
	imx8mp_clk* n = _imx8mp_clk_alloc(name);
	n->recalc_rate = 0;
	n->num_parent = n_parents;
	n->parent = parent;
	n->is_composite = 1;
	n->pre_podf = 0;
	n->post_podf = 0;
	n->anatop_base = anatop;
	n->_pll_read = 1;
	return n;
}

static uint32_t AuDeviceTreeGetFixedClockRate(const char* node_name) {
	uint32_t* node = AuDeviceTreeGetNode(node_name);
	if (!node) {
		BPrintK(BORDOISILA_ERROR, "imx8mp dtp node: %s not found \r\n", node_name);
		return 0;
	}

	uint32_t rate = AuDeviceTreeGetU32Property(node, "clock-frequency", 0);
	BPrintK(BORDOISILA_INFO, "clock rate for node : %s is %u \r\n", node_name, rate);
	return rate;
}


#define IMX8MP_SYS_PLL1_RATE_HZ 800000000UL
#define IMX8MP_SYS_PLL2_RATE_HZ 1000000000UL

static imx8mp_clk* g_sys_pll1_40m;
static imx8mp_clk* g_sys_pll1_80m;
static imx8mp_clk* g_sys_pll1_100m;
static imx8mp_clk* g_sys_pll1_133m;
static imx8mp_clk* g_sys_pll1_160m;
static imx8mp_clk* g_sys_pll1_200m;
static imx8mp_clk* g_sys_pll1_266m;
static imx8mp_clk* g_sys_pll1_400m;
static imx8mp_clk* g_sys_pll1_800m;

static imx8mp_clk* g_sys_pll2_50m;
static imx8mp_clk* g_sys_pll2_100m;
static imx8mp_clk* g_sys_pll2_125m;
static imx8mp_clk* g_sys_pll2_166m;
static imx8mp_clk* g_sys_pll2_200m;
static imx8mp_clk* g_sys_pll2_250m;
static imx8mp_clk* g_sys_pll2_333m;
static imx8mp_clk* g_sys_pll2_500m;
static imx8mp_clk* g_sys_pll2_1000m;
static imx8mp_clk* g_osc_24m;

static imx8mp_clk* g_clk_ext1;
static imx8mp_clk* g_clk_ext2;
static imx8mp_clk* g_clk_ext3;
static imx8mp_clk* g_clk_ext4;

static imx8mp_clk* g_video_pll1_out;
static imx8mp_clk* g_audio_pll1_out;

static imx8mp_clk* g_audio_pll2_out;
static imx8mp_clk* g_sys_pll3_out;

static void imx8mp_config_fixed_clock() {
	uint64_t parent = imx8mp_pll_get_parent_rate(__IMX8MP_SYS_PLL1_GEN_CTRL);

	uint64_t pll1_rate = imx8mp_pll_recalc_rate(__IMX8MP_SYS_PLL1_GEN_CTRL, parent);
	parent = imx8mp_pll_get_parent_rate(__IMX8MP_SYS_PLL2_GEN_CTRL);
	uint64_t pll2_rate = imx8mp_pll_recalc_rate(__IMX8MP_SYS_PLL2_GEN_CTRL, parent);

	parent = imx8mp_pll_get_parent_rate(__IMX8MP_SYS_PLL3_GEN_CTRL);
	uint64_t pll3_rate = imx8mp_pll_recalc_rate(__IMX8MP_SYS_PLL3_GEN_CTRL, parent);
	BPrintK(BORDOISILA_INFO, "imx8mp: configuring clock rate database, pll1 rate: %u, pll3 rate: %u \r\n", 
		pll1_rate, pll3_rate);


	/**
	 * DO NOTE: video pll1 rate and audio pll1 rate are in P-O-R (Power on Reset) values
	 * maybe, we need to configure that using CCM analog base
	 */
	parent = imx8mp_pll_get_parent_rate(__IMX8MP_VIDEO_PLL1_GEN_CTRL);
	uint64_t video_pll1_rate = imx8mp_pll_recalc_rate(__IMX8MP_VIDEO_PLL1_GEN_CTRL, parent);

	parent = imx8mp_pll_get_parent_rate(__IMX8MP_AUDIO_PLL1_GEN_CTRL);
	uint64_t audio_pll1_rate = imx8mp_pll_recalc_rate(__IMX8MP_AUDIO_PLL1_GEN_CTRL, parent);

	parent = imx8mp_pll_get_parent_rate(__IMX8MP_AUDIO_PLL2_GEN_CTRL);
	uint64_t audio_pll2_rate = imx8mp_pll_recalc_rate(__IMX8MP_AUDIO_PLL2_GEN_CTRL, parent);


	g_osc_24m = imx8mp_clk_fixed("osc_24m", 24000000);
	g_sys_pll1_800m = imx8mp_clk_fixed("sys_pll1_800m", pll1_rate);
	g_sys_pll1_400m = imx8mp_clk_fixed("sys_pll1_400m", pll1_rate / 2);
	g_sys_pll1_266m = imx8mp_clk_fixed("sys_pll1_266m", pll1_rate / 3);
	g_sys_pll1_200m = imx8mp_clk_fixed("sys_pll1_200m", pll1_rate / 4);
	g_sys_pll1_160m = imx8mp_clk_fixed("sys_pll1_160m", pll1_rate / 5);
	g_sys_pll1_133m = imx8mp_clk_fixed("sys_pll1_133m", pll1_rate / 6);
	g_sys_pll1_100m = imx8mp_clk_fixed("sys_pll1_100m", pll1_rate / 8);
	g_sys_pll1_80m = imx8mp_clk_fixed("sys_pll1_80m", pll1_rate / 10);
	g_sys_pll1_40m = imx8mp_clk_fixed("sys_pll1_40m", pll1_rate / 20);

	g_sys_pll2_1000m = imx8mp_clk_fixed("sys_pll2_1000m", pll2_rate / 1);
	g_sys_pll2_500m = imx8mp_clk_fixed("sys_pll2_500m", pll2_rate / 2);
	g_sys_pll2_333m = imx8mp_clk_fixed("sys_pll2_333m", pll2_rate / 3);
	g_sys_pll2_250m = imx8mp_clk_fixed("sys_pll2_250m", pll2_rate / 4);
	g_sys_pll2_200m = imx8mp_clk_fixed("sys_pll2_200m", pll2_rate / 5);
	g_sys_pll2_166m = imx8mp_clk_fixed("sys_pll2_166m", pll2_rate / 6);
	g_sys_pll2_125m = imx8mp_clk_fixed("sys_pll_125m", pll2_rate / 8);
	g_sys_pll2_100m = imx8mp_clk_fixed("sys_pll2_100m", pll2_rate / 10);
	g_sys_pll2_50m = imx8mp_clk_fixed("sys_pll2_50m", pll2_rate / 20);

	uint32_t ext1_hz = AuDeviceTreeGetFixedClockRate("clock-ext1");
	uint32_t ext2_hz = AuDeviceTreeGetFixedClockRate("clock-ext2");
	uint32_t ext3_hz = AuDeviceTreeGetFixedClockRate("clock-ext3");
	uint32_t ext4_hz = AuDeviceTreeGetFixedClockRate("clock-ext4");

	g_clk_ext1 = imx8mp_clk_fixed("clk_ext1", ext1_hz);
	g_clk_ext2 = imx8mp_clk_fixed("clk_ext2", ext2_hz);
	g_clk_ext3 = imx8mp_clk_fixed("clk_ext3", ext3_hz);
	g_clk_ext4 = imx8mp_clk_fixed("clk_ext4", ext4_hz);

	g_audio_pll1_out = imx8mp_clk_fixed("audio_pll1_out", audio_pll1_rate);
	g_video_pll1_out = imx8mp_clk_fixed("video_pll1_out", video_pll1_rate);
	g_audio_pll2_out = imx8mp_clk_fixed("audio_pll2_out", audio_pll2_rate);
	g_sys_pll3_out = imx8mp_clk_fixed("sys_pll3_out", pll3_rate);
}



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


static bool is_imx8mp_clk_enabled(uint32_t clk_idx) {
	volatile uint32_t* root = (volatile uint32_t*)CCM_ROOT_REG(_ccm_base, clk_idx);
	bool is_true = false;
	uint32_t rval = *root;
	if ((rval & (1u << 28)) != 0)
		is_true = true;

	return is_true;
}

static uint32_t imx8mp_clk_get_mux(uint32_t clk_idx) {
	volatile uint32_t* root = (volatile uint32_t*)CCM_ROOT_REG(_ccm_base, clk_idx);
	uint32_t rval = *root;
	return (rval >> 24) & 0x7u;
}


#define FORM_CLK_COMPOSITE(name,parent,n_parent) imx8mp_clk_composite(name,parent,n_parent)


/**
 *imx8mp_void_ccm_init -- map the ccm module
 */
void imx8mp_ccm_init() {
	AuTextOut("[imx8mp_board]: initializing clock control module (ccm) \r\n");
	_ccm_base = (uint64_t)CCM_BASE; // AuMapMMIO(CCM_BASE, 16);
	
	imx8mp_config_fixed_clock();

	/**
	 * Reference: Linux source code, clk-imx8mp.c
	 */

	static imx8mp_clk* media_axi_parents[8];
	media_axi_parents[0] = g_osc_24m;
	media_axi_parents[1] = g_sys_pll2_1000m;
	media_axi_parents[2] = g_sys_pll1_800m;
	media_axi_parents[3] = g_sys_pll3_out;
	media_axi_parents[4] = g_sys_pll1_40m;
	media_axi_parents[5] = g_audio_pll2_out;
	media_axi_parents[6] = g_clk_ext1;
	media_axi_parents[7] = g_sys_pll2_500m;
	imx8mp_clk* media_axi_clk = FORM_CLK_COMPOSITE("media_axi_axi", media_axi_parents, 8);

	static imx8mp_clk* media_apb_parents[8];
	media_apb_parents[0] = g_osc_24m;
	media_apb_parents[1] = g_sys_pll2_125m;
	media_apb_parents[2] = g_sys_pll1_800m;
	media_apb_parents[3] = g_sys_pll3_out;
	media_apb_parents[4] = g_sys_pll1_40m;
	media_apb_parents[5] = g_audio_pll2_out;
	media_apb_parents[6] = g_clk_ext1;
	media_apb_parents[7] = g_sys_pll1_133m;
	imx8mp_clk* media_apb_clk = FORM_CLK_COMPOSITE("media_apb_axi", media_apb_parents, 8);

	/** by default let's only enable HDMI + LCDIF, because we need 
	 * framebuffer output :) 
	 */

	// let's enable some clock roots by default, by setting mux/pre-podf/post-podf 
	if (is_imx8mp_clk_enabled(AHB_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "ccm ahb clock root enabled mux: %d\r\n", imx8mp_clk_get_mux(AHB_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "ccm ahb clock is not enabled \r\n");

	if (is_imx8mp_clk_enabled(HDMI_APB_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "hdmi apb clock root enabled, mux: %d \r\n", imx8mp_clk_get_mux(HDMI_APB_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "hdmi apb clock is not enabled \r\n");

	if (is_imx8mp_clk_enabled(GPU_AHB_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "GPU ahb clock root enabled , mux: %d\r\n", imx8mp_clk_get_mux(GPU_AHB_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "GPU AHB Clock is not enabled \r\n");

	if (is_imx8mp_clk_enabled(GPU_AXI_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "GPU AXI clock root enabled, mux: %d\r\n", imx8mp_clk_get_mux(GPU_AXI_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "GPU AXI Clock not enabled\r\n");

	if (is_imx8mp_clk_enabled(MEDIA_APB_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "MEDIA APB clock root enabled, mux: %d \r\n", imx8mp_clk_get_mux(MEDIA_APB_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "MEDIA APB clock not enabled \r\n");

	if (is_imx8mp_clk_enabled(MEDIA_AXI_CLK_ROOT)) {
		BPrintK(BORDOISILA_WARN, "MEDIA AXI clock enabled, mux : %d \r\n", imx8mp_clk_get_mux(MEDIA_AXI_CLK_ROOT));
	}
	else
		BPrintK(BORDOISILA_WARN, "MEDIA AXI clock not enabled \r\n");


	//and gate them 

	/** todo try setting the clr bits also */
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

