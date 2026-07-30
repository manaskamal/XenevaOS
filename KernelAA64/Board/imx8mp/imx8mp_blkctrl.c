/**
* @file imx8mp_blkctrl.c
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

#include <Board/imx8mp/imx8mp_blkctrl.h>
#include <bordoisila_bits.h>
#include <bordoisila_io.h>
#include <_null.h>
#include <Log/klog.h>

#define IMX8MP_BLK_CTRL_DDR 0x3D000000
#define IMX8MP_BLK_CTRL_HSIO 0x32F10000
#define IMX8MP_BLK_CTRL_MEDIA 0x32EC0000
#define IMX8MP_BLK_CTRLAUDIO  0x30E20000
#define IMX8MP_BLK_CTRL_VPU   0x38330000

#define BLK_SFT_RSTN  0x0
#define BLK_CLK_EN    0x4
#define BLK_MIPI_RESET_DIV  0x8

typedef struct _imx8mp_blk_map_ {
	int blk_id;
	uint32_t reg;
	uint32_t rst_mask;
	uint32_t clk_mask;
}imx8mp_blk_map;

static imx8mp_blk_map _map[10];

#define BLK_MAP_ENTRY(n,_id, _reg, _rst_mask, _clk_mask) \
      _map[n].blk_id = _id; \
      _map[n].reg = _reg; \
      _map[n].rst_mask = _rst_mask;\
      _map[n].clk_mask = _clk_mask; 


/**
 * @brief imx8mp_blkctl_init -- register the block control
 * registry, for now, only MEDIA domain is entered
 */
void imx8mp_blkctrl_init() {
	int n = 0;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_MIPI_DSI_1, 
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(0) | BORDOISILA_BIT(1),
		BORDOISILA_BIT(0) | BORDOISILA_BIT(1));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_MIPI_CSI2_1,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(2) | BORDOISILA_BIT(3),
		BORDOISILA_BIT(2) | BORDOISILA_BIT(3));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_LCDIF_1,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(4) | BORDOISILA_BIT(5) | BORDOISILA_BIT(23),
		BORDOISILA_BIT(4) | BORDOISILA_BIT(5) | BORDOISILA_BIT(23));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_ISI,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(6) | BORDOISILA_BIT(7),
		BORDOISILA_BIT(6) | BORDOISILA_BIT(7));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_MIPI_CSI2_2,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(9) | BORDOISILA_BIT(10),
		BORDOISILA_BIT(9) | BORDOISILA_BIT(10));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_LCDIF_2,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(11) | BORDOISILA_BIT(12) | BORDOISILA_BIT(24),
		BORDOISILA_BIT(11) | BORDOISILA_BIT(12) | BORDOISILA_BIT(24));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_ISP,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(16) | BORDOISILA_BIT(17) | BORDOISILA_BIT(18),
		BORDOISILA_BIT(16) | BORDOISILA_BIT(17) | BORDOISILA_BIT(18));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_DWE,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(19) | BORDOISILA_BIT(20) | BORDOISILA_BIT(21),
		BORDOISILA_BIT(19) | BORDOISILA_BIT(20) | BORDOISILA_BIT(21));

	n++;

	BLK_MAP_ENTRY(n,
		IMX8MP_MEDIABLK_PD_MIPI_DSI_2,
		IMX8MP_BLK_CTRL_MEDIA,
		BORDOISILA_BIT(22), BORDOISILA_BIT(22));

}

static imx8mp_blk_map* imx8mp_blkctl_find(uint32_t id) {
	for (int i = 0; i < 10; i++) {
		if (_map[i].blk_id == id) {
			return &_map[i];
		}
	}

	return NULL;
}

int imx8mp_blkctl_powerup(uint32_t id) {
	imx8mp_blk_map* domain = imx8mp_blkctl_find(id);
	if (!domain) {
		BPrintK(BORDOISILA_ERROR, "imx8mp-blk-ctl id : %d, not found \r\n", id);
		return 1;
	}

	BPrintK(BORDOISILA_DEBUG, "BLK CTRL power up : %x \r\n", domain->reg);
	uint32_t bus_clk = _bordoisila_readl(domain->reg + BLK_CLK_EN);
	BPrintK(BORDOISILA_DEBUG, "blkctl bus val : %x, addr : %x \r\n", (domain->reg + BLK_CLK_EN));
	bus_clk |= BORDOISILA_BIT(8);
	_bordoisila_writel(bus_clk, domain->reg + BLK_CLK_EN);


	uint32_t bus_rstn = _bordoisila_readl(domain->reg + BLK_SFT_RSTN);
	BPrintK(BORDOISILA_DEBUG, "bus rstn : %x , addr : %x \r\n", bus_rstn, (domain->reg + BLK_SFT_RSTN));
	bus_rstn |= BORDOISILA_BIT(8);
	_bordoisila_writel(bus_rstn, domain->reg + BLK_SFT_RSTN);


	/** device reset assert **/
	uint32_t rstn = _bordoisila_readl(domain->reg + BLK_SFT_RSTN);
	BPrintK(BORDOISILA_DEBUG, "rstn : %x , addr : %x \r\n", rstn, (domain->reg + BLK_SFT_RSTN));
	rstn &= ~domain->rst_mask;
	_bordoisila_writel(rstn,domain->reg + BLK_SFT_RSTN);

	uint32_t clken = _bordoisila_readl(domain->reg + BLK_CLK_EN);
	clken |= domain->clk_mask;
	_bordoisila_writel(clken,domain->reg + BLK_CLK_EN);

	for (int i = 0; i < 100000; i++)
		;


	BPrintK(BORDOISILA_INFO, "imx8mp-blkctrl: id : %d powered properly \r\n", domain->blk_id);
}

int imx8mp_blkctrl_release_reset(uint32_t id) {
	imx8mp_blk_map* domain = imx8mp_blkctl_find(id);
	if (!domain) {
		BPrintK(BORDOISILA_ERROR, "imx8mp-blk-ctl id : %d, not found \r\n", id);
		return 1;
	}

	uint32_t rstn = _bordoisila_readl(domain->reg + BLK_SFT_RSTN);
	rstn |= domain->rst_mask;
	_bordoisila_writel(rstn, domain->reg + BLK_SFT_RSTN);

	BPrintK(BORDOISILA_INFO, "imx8mp-blkctrl: id : %d reset deasserted \r\n", domain->blk_id);

}